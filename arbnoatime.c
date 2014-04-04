/*
 * arbnoatime.c
 * A daemon to remount volumes as noatime.
 */

/* Installation:
cc -framework CoreFoundation -framework DiskArbitration arbnoatime.c -o /Library/PrivilegedHelperTools/net.clockish.arbnoatime
cat <<HERE > /Library/LaunchDaemons/net.clockish.arbnoatime.plist
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>Label</key>
	<string>net.clockish.arbnoatime</string>
	<key>ProgramArguments</key>
	<array>
		<string>/Library/PrivilegedHelperTools/net.clockish.arbnoatime</string>
	</array>
	<key>KeepAlive</key>
	<true/>
	<key>RunAtLoad</key>
	<true/>
</dict>
</plist>
HERE
chown root:wheel /Library/PrivilegedHelperTools/net.clockish.arbnoatime /Library/LaunchDaemons/net.clockish.arbnoatime.plist
chmod 755 /Library/PrivilegedHelperTools/net.clockish.arbnoatime
chmod 644 /Library/LaunchDaemons/net.clockish.arbnoatime.plist
*/

#include <unistd.h>
#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>

void mountunoatime(DADiskRef disk, void *context) {
	UInt8 path[4096]; // As part of a longstanding API problem, have to just hope this is big enough.
    if(!DADiskGetBSDName(disk)) return; //Chicken out if the disk doesn't look normal.
    
	CFDictionaryRef desc = DADiskCopyDescription(disk);
	CFBooleanRef writable = CFDictionaryGetValue(desc, CFSTR("DAMediaWritable"));
    Boolean locked = writable && !CFBooleanGetValue(writable);
	CFURLRef url = CFDictionaryGetValue(desc, CFSTR("DAVolumePath"));
    
    // We only care about actually mounted volumes that are from potentially writeable media
	if(!locked && url && CFURLGetFileSystemRepresentation(url, true, path, sizeof(path))) {
		pid_t pid = fork();
		if(!pid) {
		    execl("/sbin/mount", "mount", "-uo", "noatime", path, NULL);
		    fprintf(stderr, "Failed to exec /sbin/mount\n");
		    _exit(-1);
		}
        
		int status;
		waitpid(pid, &status, 0);
		if(!WIFEXITED(status) || WEXITSTATUS(status)) {
		    fprintf(stderr, "Failed to mount -uo noatime %s\n", path);
		}
	}
    
	CFRelease(desc);
}

int main(int argc, char *argv[]) {
	DASessionRef session = DASessionCreate(kCFAllocatorDefault);
	DARegisterDiskAppearedCallback(session, NULL, mountunoatime, NULL);
	DASessionScheduleWithRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	CFRunLoopRun();
	return 0;
}
