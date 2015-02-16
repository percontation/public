#!/usr/bin/env python
import sys, subprocess, math

if len(sys.argv) == 1:
  print >> sys.stderr, "Usage: %s [uUlLdDrR]*\n" % sys.argv[0]
  print >> sys.stderr, "Moves and resizes the active X window to a subdivision of the screen.\n"
  print >> sys.stderr, "The initial target is the whole desktop, and the the input argument is"
  print >> sys.stderr, "iterated over to subdivide the target space to the [u]pper half, [l]eft half,"
  print >> sys.stderr, "[d]own half, or [r]ight half. For example, `./tile.py ul` and `./tile.py lu`"
  print >> sys.stderr, "both move+resize the active window to the upper left quadrant of the desktop"
  print >> sys.stderr, "space. Capitals ULDR can be used to subdivide to 2/3rds: e.g. `./tile.py Lr`"
  print >> sys.stderr, "will make the window full height and 1/3 width in the middle of the screen.\n"
  print >> sys.stderr, "Requires `apt-get install x11-utils xdotool wmctrl`\n"
  exit(64)

def sh(s):
  return subprocess.check_output(('sh', '-c', s))

if sh("which xprop xdotool wmctrl > /dev/null && echo yes").strip() != "yes":
  print >> sys.stderr, "Missing required dependencies. Try `apt-get install x11-utils xdotool wmctrl`"
  exit(-1)

x,y,w,h = map(float, sh(r"""wmctrl -d | perl -ne '/[0-9]+ *\*.*WA: ([0-9]+),([0-9]+) ([0-9]+)x([0-9]+)/ && print "$1 $2 $3 $4"'""").split(" "))

for c in ''.join(sys.argv[1:]):
  if   c == 'u': h = h*1/2
  if   c == 'U': h = h*2/3
  elif c == 'l': w = w*1/2
  elif c == 'L': w = w*2/3
  elif c == 'd': h = h*1/2; y += h
  elif c == 'D': h = h*2/3; y += h/2
  elif c == 'r': w = w*1/2; x += w
  elif c == 'R': w = w*2/3; x += w/2

l,r,t,b = map(int, sh('xprop _NET_FRAME_EXTENTS -id $(xdotool getactivewindow)').split('=')[1].split(','))
x += l
y += t
w -= l+r
h -= t+b

def INT(x): return int(math.ceil(x))

# NorthWest (1), North (2), NorthEast (3), West (4), Center (5), East (6), SouthWest (7), South (8), SouthEast (9) and Static (10)
sh("wmctrl -r :ACTIVE: -e 10,"+",".join(map(str, (INT(x),INT(y),int(w),int(h)))))

