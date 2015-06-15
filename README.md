public
======

Stuff I wish I never needed to write.

OS X
----

Best OS.

#### arbnoatime

An OS X launch daemon that remounts all volumes with noatime.

#### keystroke

A slightly janky OS X program for simulating keystrokes. Useful for hacks.

#### Link Defuse

A Safari extension for unscrewing Google and Facebook links. It used to be necessary to keep Safari history sane, and might still nix a bit of Google & Facebook tracking.

#### Finder Slob

A ~/Library/LaunchAgents agent that periodically closes excess Finder windows.

#### portup.sh

A script to upgrade MacPorts ports. The primary feature is mounting a ramdisk to build on.

#### Unquarantine.workflow

An OS X ~/Library/Services script to remove the quarantined xattr from files & directory trees. Useful for mollifying Gatekeeper by removing the "downloaded" stigma from .app bundles.

Linux
-----

Rather, "not just OS X".

#### tc_limiting.sh

A script that reconfigures Linux qdiscs to a particular configuration, that throttles outbound nginx traffic.

#### tile.py

A script for manually tiling X windows. You bind various invocations of the script to hotkeys to send the frontmost window to a particular subdivision of the screen. Works for xfwm4 (i.e. XFCE) and possibly others.

#### zshrc

More-or-less cross platform rc file. Pretty old school, and mostly not zsh specific.
