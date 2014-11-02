#!/bin/bash
#
# A script to update MacPorts ports.
# Attaches a ramdisk while building.
# Has a feature whereby "large" ports are bin-only.
#

# Path to where MacPorts builds things
MPBUILD="/opt/local/var/macports/build"

# Regex matching ports to be installed bin-only (without 'all' flag),
# or built off the ramdisk (with 'all' flag).
LARGE='llvm-.*|clang-.*|gcc..|pypy|gtk.|boost|rust'

# Size of ramdisk in MB; must be large enough to build everything
# not in the above large list!
RAMDISKMB=$((3 * 1024))

if [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
	echo "Usage: $0 [all | port-regex]"
	echo "A script for updating MacPorts ports; the optional argument"
	echo "specifies which ports should be upgraded:"
	echo ""
	echo "'all' arg: outdated ports"
	echo "other arg: outdated ports that match the given regex"
	echo "no args:   outdated ports, but binary-only for ports matching:"
	echo "	$LARGE"
	exit 0
fi

if [ "$UID" != "0" ]; then
	echo "Sudoing..."
	exec sudo "$0" "$@"
	exit 1
fi

port selfupdate || port sync
OUTDATED=`port echo outdated | awk '{print $1}'`

if [ -n "$1" ] && [ "$1" != "all" ]; then
	# Filter outdated by the argument
	OUTDATED=`echo "$OUTDATED" | grep -xEe "$1"`
fi

# Split the normal ports from the "large" ports
UPDATE=` echo "$OUTDATED" | grep -v -xEe "$LARGE" | tr -s '\n' ' '`
BINONLY=`echo "$OUTDATED" | grep    -xEe "$LARGE" | tr -s '\n' ' '`

# Update normal ports, building on a ramdisk
if [ -n "$(echo "$UPDATE" | tr -d ' ')" ]; then
	echo "Will update: $UPDATE"
	
	RAMDISK=`hdiutil attach -nomount ram://$((1024*1024/512*($RAMDISKMB))) | awk '{print $1}'` && \
	(
		trap "umount -f '$RAMDISK'; hdiutil detach '$RAMDISK'; exit" 0 1 2 15
		newfs_hfs -sv "macports build" "$RAMDISK" && \
		mount -t hfs -wo nodev,union,nobrowse "$RAMDISK" "$MPBUILD" && \
		caffeinate -s port -pcun upgrade $UPDATE
	) || echo 'Failed to upgrade ports. Try again maybe?'
fi

# Update the "large" ports, usually binary-only
if [ -n "$(echo "$BINONLY" | tr -d ' ')" ]; then
	echo "Will update: $BINONLY"
	if [ -n "$1" ]; then
		# Not binary only when given the 'all' flag or a port-regex.
		caffeinate -s port -pcu upgrade $BINONLY
	else
		# Binary only in the default case
		echo "Don't mind errors in the following:"
		caffeinate -s port -pbcu upgrade $BINONLY
	fi
fi

# Print out names of inactive and leaf ports.

INACT=`port echo inactive  | awk '{print $1}' | tr -s '\n' ' '`
if [ -n "$(echo "$INACT" | tr -d ' ')" ]; then
  echo "Inactive: $INACT"
fi

LEAVES=`port echo leaves | awk '{print $1}' | tr -s '\n' ' '`
if [ -n "$(echo "$LEAVES" | tr -d ' ')" ]; then
  echo "Leaves: $LEAVES"
fi
