#! /bin/bash
#
# Wrapper script for mysql_config to support multilib
#
#

# This command respects setarch, works on OL6/RHEL6 and later
bits=$(rpm --eval %__isa_bits)

case $bits in
    32|64) ;;
        *) bits=unknown ;;
esac

# Try mapping by uname if rpm command failed
if [ "$bits" = "unknown" ] ; then
    arch=$(uname -m)
    case $arch in
	x86_64|ppc64|ppc64le|aarch64|s390x|sparc64) bits=64 ;;
	i386|i486|i586|i686|pentium3|pentium4|athlon|ppc|s390|sparc) bits=32 ;;
	*) bits=unknown ;;
    esac
fi

if [ "$bits" == "unknown" ] ; then
    echo "$0: error: failed to determine isa bits on your arch."
    exit 1
fi

if [ -x /usr/bin/mysql_config-$bits ] ; then
    /usr/bin/mysql_config-$bits "$@"
else
    echo "$0: error: needed binary: /usr/bin/mysql_config-$bits is missing. Please check your MySQL installation."
    exit 1
fi
