LSWIFI(1) - General Commands Manual

# NAME

**lswifi** - scans for wifi/wlan networks and list them as CSV.

# SYNOPSIS

**lswifi**
\[**-v**]
\[**-s**&nbsp;*separator*]
\[**-r**&nbsp;*record-separator*]

# DESCRIPTION

The
**lswifi**
works like 'ifconfig \* scan' and outputs the found wifi networks in an
easy-to-parse CSV format.
It tries out all existing interfaces and skips those who cannot run the 'scan'
command.

## Options

**-s** *separator*

> Set the separator between the given fields (Default: Tab)

**-r** *record-separator*

> Set the separator between the given networks (Default: Newline)

**-v**

> Verbose output, mainly meant for debuggin

## Output Columns

Interface

> The Interface on which the network was found

Connected

>  '1' if the Network is already connected on that interface. '0' otherwise.

Network Name

> The visible Name of the Network

Network SSID

> The SSID of the network in human readable format

Signal Strength

> The Signal strength in the unit of the following column.

Signal Strength Unit

> The Unit in which the Signal strength is expressed (% or dBm).

# EXIT STATUS

Always null. Errors are silently skipped.

# SEE ALSO

ifconfig(1)

# HISTORY

Built to simplify building a graphical OpenBSD Wifi Manager.

# AUTHORS

Andreas Koerner &lt;[andi@jaak.de](mailto:andi@jaak.de)&gt;

# SECURITY CONSIDERATIONS

Needs to be run as root to be allowed to do wifi scanning. pledges itself before
writing any output. Yet you better audit the code before you run it.
The Code is just 300 lines.

OpenBSD 6.3 - July 16, 2018
