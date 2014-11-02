# public

Misc scripts, mostly the mundane products of me living on OS X.

## arbnoatime

An OS X launch daemon that remounts all volumes with noatime. Some people claim various benefits to SSD or rotational media.

## keystroke

A slightly janky OS X program for simulating keystrokes. Useful for one-off hacks, and works well with scripts.

## Link Defuse

A Safari extension for undoing the silliness applied to Google and Facebook links. This formerly fixed issues preventing Safari's history from working correctly, but now it just nullfies some Google and Facebook tracking.

## Finder Slob

A ~/Library/LaunchAgents agent that periodically closes excess Finder windows.

## portup.sh

A script to upgrade MacPorts ports. The primary feature is mounting a ramdisk to build on.

## tc_limiting.sh

A script that reconfigures Linux qdiscs to a particular configuration, that throttles outbound nginx traffic.

## Unquarantine.workflow

An OS X ~/Library/Services script to remove the quarantined xattr from files & directory trees. (This xattr is what makes Gatekeeper, in its default configuration, refuse to launch unsigned downloaded applications).

## zshrc

My more-or-less cross platform rc file. Mostly not zsh specific; I don't zsh too hard. There's one or two fun tricks in there.
