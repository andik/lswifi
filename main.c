/*
 * Copyright (c) 2018 Andreas Koerner <andi@jaak.de>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/queue.h>

#include <ifaddrs.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet6/in6_var.h>

#include <net80211/ieee80211.h>
#include <net80211/ieee80211_ioctl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>

/* ---------------------------------------------------------------------- */

/* Structures
   ---------- */


/* all relevant data for a single wifi network */
SLIST_HEAD(wifidat_head, wifidat) interfaces;
struct wifidat {
	SLIST_ENTRY(wifidat) elems;
	const char* interface;            /* interface name */
	struct ieee80211_nodereq_all na;  /* result for ioctl(SCAN) */
	struct ieee80211_nodereq nr[512]; /* list of all networks */

	struct ieee80211_nwid nwid;       /* connected network name */
	struct ieee80211_bssid bssid;     /* connected netword id */
};
	
/* ---------------------------------------------------------------------- */

/* Globals
   ------- */
	 
/* Command line options */
char* sep;   /* field separator  */
char* rsep;  /* record separator */
int verbose; /* verbose output   */

/* ---------------------------------------------------------------------- */

void usage() {
	fprintf(stderr, "usage: lswifi [-v] [-s separator]\n");
	exit(-1);
}

/* ---------------------------------------------------------------------- */

int network_name_is_sane(const u_char* name, int len)
{
	int i;

	if (len > IEEE80211_NWID_LEN)
		return 0;

	for (; i < len; i++)
		if (name[i] & 0x80 || !isprint(name[i])) 
			return 0;

	return 1;
}

/* ---------------------------------------------------------------------- */

void print_interface_data(struct wifidat* data)
{
	int i, len, connected;
	struct ieee80211_nodereq* network;
	
	for (i = 0; i < data->na.na_nodes; i++) {
		network = &data->nr[i];
		
		/* Checks 
		   ------ */

		/* Network name (not zero ended, sadly) */
		len = network->nr_nwid_len;
		if (!network_name_is_sane(network->nr_nwid, len)) 
			return; /* Ensure only sane network names are printed... */
		
		/* connected to that network? */
		connected = (len == data->nwid.i_len
			&& memcmp(network->nr_nwid, data->nwid.i_nwid, len) == 0
			&& memcmp(network->nr_bssid, data->bssid.i_bssid, IEEE80211_ADDR_LEN) == 0
		);
	
		
		/* Printing 
			 -------- */
		
		/* interface */
		printf("%s", data->interface);
		printf("%s", sep);
			
		/* connected to the network */
		printf("%i", connected ? 1 : 0);
		printf("%s", sep);
		
		/* SSID */
		printf("%s", ether_ntoa((struct ether_addr*)network->nr_bssid));
		printf("%s", sep);
		
		/* Signal Quality - separated in value and unit (parsing sucks ;-) */
		if (network->nr_max_rssi)
			printf("%u%s%%", IEEE80211_NODEREQ_RSSI(network), sep);
		else
			printf("%d%sdBm", network->nr_rssi, sep);
		printf("%s", sep);
		
		/* Network Name (comes last as it has different lengths) */
		printf("%.*s", len, network->nr_nwid);
		
		/* End of Line/Record */
		printf("%s", rsep);
	}
}

/* ---------------------------------------------------------------------- */

int query_interface(const char* if_name, struct wifidat* data)
{
	int sock;
	struct ifreq ifr;     /* request - input to/output from ioctl */
	int inwid, ibssid;
	
	if(verbose)
		printf("# interface %s\n", if_name);
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	
	/* store interface name for later */
	data->interface = if_name;
	
	/* check if valid and connected wifi network */
	
	bzero(&ifr, sizeof(ifr));
	ifr.ifr_data = (caddr_t)&data->nwid;
	strlcpy(ifr.ifr_name, if_name, sizeof(ifr.ifr_name));
	inwid = ioctl(sock, SIOCG80211NWID, (caddr_t)&ifr);

	bzero(&data->bssid, sizeof(data->bssid));
	strlcpy(data->bssid.i_name, if_name, sizeof(data->bssid.i_name));
	ibssid = ioctl(sock, SIOCG80211BSSID, &data->bssid);

	/* check if any ieee80211 option is active */
	if (inwid != 0 && ibssid != 0)
		return -1;
	
	/* copy the name of the interface to the request struct */
	bzero(&ifr, sizeof(ifr));
	strlcpy(ifr.ifr_name, if_name, sizeof(ifr.ifr_name));
	
	/* scan or, if error, return (the interface is no wifi...)*/
	if (ioctl(sock, SIOCS80211SCAN, (caddr_t)&ifr) != 0)
		return -1;
	
	/* get the scan result */
	bzero(&data->na, sizeof(data->na));
	bzero(&data->nr, sizeof(data->nr));
	data->na.na_node = data->nr;
	data->na.na_size = sizeof(data->nr);
	strlcpy(data->na.na_ifname, if_name, sizeof(data->na.na_ifname));

	if (ioctl(sock, SIOCG80211ALLNODES, &data->na) != 0) {
		warn("SIOCG80211ALLNODES");
		return -1;
	}

	/* next interface if nothing was found. */
	if (!data->na.na_nodes)
		return -1;
	
	close(sock);
	
	return 0;
}

/* ---------------------------------------------------------------------- */

int main(int argc, char** argv)
{
	/*
		
		the task of scanning and printing wifi networks is split two parts.
		A) getting the data
		B) evaluating/printing the data
		This is done, so we can pledge() before evaluation (found no useful
		pledge() which we could use from the beginning).
		This splitting creates a relevant overhead, because we need to
		allocate some data structures (the wifidat list).
		I'd rather avoided this, but it's impossible.
	
	*/
	
	struct ifaddrs *ifap; /* all existing interfaces */
	struct ifaddrs *ifa;  /* current interface in iteration over ifap */
	int i;
	char ch;
	struct wifidat* data;
	
	/* Command line Options 
	   -------------------- */
	
	verbose = 0;
	sep = "\t";
	rsep = "\n";
	

	while ((ch = getopt(argc, argv, "s:r:v")) != -1) {
		switch (ch) {
			case 's':
				sep = optarg;
				break;
			case 'r':
				rsep = optarg;
				break;
			case 'v':
				verbose = 1;
				break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	
	/* Action
	   ------ */
	
	SLIST_INIT(&interfaces);
	
	/* iterate over interfaces */
	if (getifaddrs(&ifap) != 0)
		err(1, "getifaddrs");

	/* query for interfaces and networks */
	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
		data = malloc(sizeof(struct wifidat));
		
		if(query_interface(ifa->ifa_name, data) == -1)
		{
			/* skip if this is no wifi interface or had errors */
			free(data);
			continue;
		}
		else
		{
			SLIST_INSERT_HEAD(&interfaces, data, elems);
		}
	}
	
	/* secure before handling data */
	pledge("stdio", NULL);

	/* Print the Result of the Scan */	
	SLIST_FOREACH(data, &interfaces, elems)
	{
		print_interface_data(data);
	}
	
	return 0;
}
