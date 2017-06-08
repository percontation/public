/*/../usr/bin/true; set -eux; cd "`dirname "$0"`"

# Reverses the scroll direction of mouse wheels.
# Kinda like https://github.com/pilotmoon/Scroll-Reverser but without options, and uses less CPU.
# Self-install with `sh scrollrev.c`

rm -f $HOME/Library/LaunchAgents/net.clockish.scrollrev $HOME/Library/LaunchAgents/net.clockish.scrollrev.plist
cc -Os -framework CoreFoundation -framework CoreGraphics scrollrev.c -o $HOME/Library/LaunchAgents/net.clockish.scrollrev
strip $HOME/Library/LaunchAgents/net.clockish.scrollrev
cat <<HERE > $HOME/Library/LaunchAgents/net.clockish.scrollrev.plist
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>Label</key>
	<string>net.clockish.scrollrev</string>
	<key>ProgramArguments</key>
	<array>
		<string>$HOME/Library/LaunchAgents/net.clockish.scrollrev</string>
	</array>
	<key>KeepAlive</key>
	<true/>
	<key>RunAtLoad</key>
	<true/>
</dict>
</plist>
HERE
exit 0
*/

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>

static CGEventRef callback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
  if(CGEventGetIntegerValueField(event, kCGScrollWheelEventIsContinuous))
    return event;
  CGEventSetDoubleValueField(event, kCGScrollWheelEventFixedPtDeltaAxis1, -CGEventGetDoubleValueField(event, kCGScrollWheelEventFixedPtDeltaAxis1));
  CGEventSetDoubleValueField(event, kCGScrollWheelEventFixedPtDeltaAxis2, -CGEventGetDoubleValueField(event, kCGScrollWheelEventFixedPtDeltaAxis2));
  return event;
}

int main(int argc, char *argv[]) {
  CGEventMask mask = CGEventMaskBit(kCGEventScrollWheel);
  CFMachPortRef port = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault, mask, &callback, NULL);
  if(port == NULL)
    return -1;
  CFRunLoopSourceRef source = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, port, 0);
  CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
  CGEventTapEnable(port, true);
  CFRunLoopRun();
  return 1;
}
