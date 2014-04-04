#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

useconds_t keylag = 1000;
useconds_t waitkp = 10000;

void usage(int exitcode) {
	fflush(stderr);
	fprintf(stderr, "Usage: keystroke [-e keys] [-c code1,code2,...] [-s sleep]\n");
	fprintf(stderr, "         [-m mod1,mod2,... -? arg] [-d(e|c) arg] [-u(e|c) arg] [-ud] [-uu]\n");
	fprintf(stderr, "         [-w wait] [-l keylag] [-p pid] ...\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Utility for simulating keystrokes. Assumes a US keyboard layout, sorry.\n");
	fprintf(stderr, "Args are commands processed from left to right.\n");
	fprintf(stderr, "\t-c codes  Presses the comma-delimited numeric key codes, or key names.\n");
	fprintf(stderr, "\t            The HIToolbox Events.h key names, sans prefixes, work, along\n");
	fprintf(stderr, "\t            with some abbreviations. Case is insensitive.\n");
	fprintf(stderr, "\t-e string Types out the characters in the string, with delays.\n");
	fprintf(stderr, "\t-l num    Change the length of time between a key down and key up event to\n");
	fprintf(stderr, "\t            num milliseconds. The default is %llums.\n", (unsigned long long)keylag);
	fprintf(stderr, "\t-m mod    Holds down the comma delimited key codes for the next argument.\n");
	fprintf(stderr, "\t            Useful for modifier keys.\n");
	fprintf(stderr, "\t-p pid    From now on, only direct events to the given process.\n");
	fprintf(stderr, "\t            Use -p 0 to restore sending to any process.\n");
	fprintf(stderr, "\t-s num    Sleep for num seconds before moving on.\n");
	fprintf(stderr, "\t-w num    Change the delay between key presses to num milliseconds. The\n");
	fprintf(stderr, "\t            delay is after each press in '-e' or '-c'. The default is %llu.\n", (unsigned long long)waitkp);
	fprintf(stderr, "\t-d? arg   Prefixing an arg with d sends just a key down event.\n");
	fprintf(stderr, "\t-u? arg   Prefixing an arg with u sends just a key up event.\n");
	fprintf(stderr, "\t-ud       Lift the keys held down by the previous -d? action. Repeating\n");
	fprintf(stderr, "\t            this works like an undo-stack.\n");
	fprintf(stderr, "\t-uu       Release all held keys.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Notes:\n");
	fprintf(stderr, "Use -dc mod to hold a modifier key; -m mod -de keys will only briefly press mod,\n");
	fprintf(stderr, "while holding down the keys. Holding keys down for a brief period can be done\n");
	fprintf(stderr, "with -m codes -s 1.5\n");
	exit(exitcode);
}

int fsleep(double seconds) {
	double sec;
	double frac = modf(seconds, &sec);
	struct timespec ts = {.tv_sec = sec, .tv_nsec = frac*1e9};
	return nanosleep(&ts, NULL);
}

double strtoseconds(char* str) {
	char *end;
	double x = strtod(str, &end);
	if(!end || !*end) return x;
	if(!strcasecmp(end, "s"))  return x;
	if(!strcasecmp(end, "ms")) return x/1000;
	if(!strcasecmp(end, "us")) return x/1000000;
	if(!strcasecmp(end, "ns")) return x/1000000000;
	if(!strcasecmp(end, "m"))  return x*60;
	if(!strcasecmp(end, "h"))  return x*3600;
	fprintf(stderr, "Error: Bad time duration %s\n", str);
	fprintf(stderr, "Recognized suffixes are h, m, s, ms, us, and ns.\n");
	usage(-1);
}

// CGKeyCode, High word: Nonzero if shift.
struct KeyCode {
	CGKeyCode code:16;
	enum {
		KeyCodeShift = 1<<0,
		KeyCodeBeep  = 1<<1,
	} flags:16;
};

struct KeyCode keyCodeForAscii(char c) {
	struct KeyCode ret = {0xFFFF, 0};
	switch(c) {
		default: ret.code = 0xFFFF; break;
#define S() (ret.flags = KeyCodeShift)
		case 'A': S(); case 'a': ret.code = kVK_ANSI_A; break;
		case 'B': S(); case 'b': ret.code = kVK_ANSI_B; break;
		case 'C': S(); case 'c': ret.code = kVK_ANSI_C; break;
		case 'D': S(); case 'd': ret.code = kVK_ANSI_D; break;
		case 'E': S(); case 'e': ret.code = kVK_ANSI_E; break;
		case 'F': S(); case 'f': ret.code = kVK_ANSI_F; break;
		case 'G': S(); case 'g': ret.code = kVK_ANSI_G; break;
		case 'H': S(); case 'h': ret.code = kVK_ANSI_H; break;
		case 'I': S(); case 'i': ret.code = kVK_ANSI_I; break;
		case 'J': S(); case 'j': ret.code = kVK_ANSI_J; break;
		case 'K': S(); case 'k': ret.code = kVK_ANSI_K; break;
		case 'L': S(); case 'l': ret.code = kVK_ANSI_L; break;
		case 'M': S(); case 'm': ret.code = kVK_ANSI_M; break;
		case 'N': S(); case 'n': ret.code = kVK_ANSI_N; break;
		case 'O': S(); case 'o': ret.code = kVK_ANSI_O; break;
		case 'P': S(); case 'p': ret.code = kVK_ANSI_P; break;
		case 'Q': S(); case 'q': ret.code = kVK_ANSI_Q; break;
		case 'R': S(); case 'r': ret.code = kVK_ANSI_R; break;
		case 'S': S(); case 's': ret.code = kVK_ANSI_S; break;
		case 'T': S(); case 't': ret.code = kVK_ANSI_T; break;
		case 'U': S(); case 'u': ret.code = kVK_ANSI_U; break;
		case 'V': S(); case 'v': ret.code = kVK_ANSI_V; break;
		case 'W': S(); case 'w': ret.code = kVK_ANSI_W; break;
		case 'X': S(); case 'x': ret.code = kVK_ANSI_X; break;
		case 'Y': S(); case 'y': ret.code = kVK_ANSI_Y; break;
		case 'Z': S(); case 'z': ret.code = kVK_ANSI_Z; break;
		case ')': S(); case '0': ret.code = kVK_ANSI_0; break;
		case '!': S(); case '1': ret.code = kVK_ANSI_1; break;
		case '@': S(); case '2': ret.code = kVK_ANSI_2; break;
		case '#': S(); case '3': ret.code = kVK_ANSI_3; break;
		case '$': S(); case '4': ret.code = kVK_ANSI_4; break;
		case '%': S(); case '5': ret.code = kVK_ANSI_5; break;
		case '^': S(); case '6': ret.code = kVK_ANSI_6; break;
		case '&': S(); case '7': ret.code = kVK_ANSI_7; break;
		case '*': S(); case '8': ret.code = kVK_ANSI_8; break;
		case '(': S(); case '9': ret.code = kVK_ANSI_9; break;
		case '_': S(); case '-': ret.code = kVK_ANSI_Minus;        break;
		case '+': S(); case '=': ret.code = kVK_ANSI_Equal;        break;
		case '~': S(); case '`': ret.code = kVK_ANSI_Grave;        break;
		case '{': S(); case '[': ret.code = kVK_ANSI_LeftBracket;  break;
		case '}': S(); case ']': ret.code = kVK_ANSI_RightBracket; break;
		case '|': S(); case '\\':ret.code = kVK_ANSI_Backslash;    break;
		case ':': S(); case ';': ret.code = kVK_ANSI_Semicolon;    break;
		case '"': S(); case '\'':ret.code = kVK_ANSI_Quote;        break;
		case '<': S(); case ',': ret.code = kVK_ANSI_Comma;        break;
		case '>': S(); case '.': ret.code = kVK_ANSI_Period;       break;
		case '?': S(); case '/': ret.code = kVK_ANSI_Slash;        break;
#undef S
		case '\n': case '\r': ret.code = kVK_Return; break;
		case '\t':   ret.code = kVK_Tab;           break;
		case ' ':    ret.code = kVK_Space;         break;
		case '\x08': ret.code = kVK_Delete;        break;
		case 127:    ret.code = kVK_ForwardDelete; break;

		case kHomeCharCode:        ret.code = kVK_Home;             break;
		case kEndCharCode:         ret.code = kVK_End;              break;
		case kHelpCharCode:        ret.code = kVK_Help;             break;
		case kPageUpCharCode:      ret.code = kVK_PageUp;           break;
		case kPageDownCharCode:    ret.code = kVK_PageDown;         break;
		case kEscapeCharCode:      ret.code = kVK_Escape;           break;
		case kLeftArrowCharCode:   ret.code = kVK_LeftArrow;        break;
		case kRightArrowCharCode:  ret.code = kVK_RightArrow;       break;
		case kDownArrowCharCode:   ret.code = kVK_DownArrow;        break;
		case kUpArrowCharCode:     ret.code = kVK_UpArrow;          break;
		case kEnterCharCode:       ret.code = kVK_ANSI_KeypadEnter; break;
		case kFunctionKeyCharCode: ret.code = kVK_Function;         break;
		
		// Eh, why not?
		case '\a': ret.code = 0xFFFF; ret.flags = KeyCodeBeep; break;
		case 17: ret.code = kVK_Command;             break;
		case 18: ret.code = kVK_Shift;               break;
		case 19: ret.code = kVK_Option;              break;
		case 20: ret.code = kVK_Control;             break;
		case '\x80'+18: ret.code = kVK_RightShift;     break;
		case '\x80'+19: ret.code = kVK_RightOption;    break;
		case '\x80'+20: ret.code = kVK_RightControl;   break;
		case '\x80'+21: ret.code = kVK_CapsLock;       break;
		case '\xe0': ret.code = kVK_F1;                    break;
		case '\xe1': ret.code = kVK_F2;                    break;
		case '\xe2': ret.code = kVK_F3;                    break;
		case '\xe3': ret.code = kVK_F4;                    break;
		case '\xe4': ret.code = kVK_F5;                    break;
		case '\xe5': ret.code = kVK_F6;                    break;
		case '\xe6': ret.code = kVK_F7;                    break;
		case '\xe7': ret.code = kVK_F8;                    break;
		case '\xe8': ret.code = kVK_F9;                    break;
		case '\xe9': ret.code = kVK_F10;                   break;
		case '\xea': ret.code = kVK_F11;                   break;
		case '\xeb': ret.code = kVK_F12;                   break;
		case '\xec': ret.code = kVK_F13;                   break;
		case '\xed': ret.code = kVK_F14;                   break;
		case '\xee': ret.code = kVK_F15;                   break;
		case '\xef': ret.code = kVK_F16;                   break;
		case '\xf0': ret.code = kVK_F17;                   break;
		case '\xf1': ret.code = kVK_F18;                   break;
		case '\xf2': ret.code = kVK_F19;                   break;
		case '\xf3': ret.code = kVK_F20;                   break;
		case '\xd0': ret.code = kVK_Mute;                  break;
		case '\xd1': ret.code = kVK_VolumeUp;              break;
		case '\xd2': ret.code = kVK_VolumeDown;            break;
		case '\xc0': ret.code = kVK_ANSI_KeypadDecimal;    break;
		case '\xc1': ret.code = kVK_ANSI_KeypadMultiply;   break;
		case '\xc2': ret.code = kVK_ANSI_KeypadPlus;       break;
		case '\xc3': ret.code = kVK_ANSI_KeypadClear;      break;
		case '\xc4': ret.code = kVK_ANSI_KeypadDivide;     break;
		case '\xc5': ret.code = kVK_ANSI_KeypadMinus;      break;
		case '\xc6': ret.code = kVK_ANSI_KeypadEquals;     break;
		case '\x80'+'0': ret.code = kVK_ANSI_Keypad0;  break;
		case '\x80'+'1': ret.code = kVK_ANSI_Keypad1;  break;
		case '\x80'+'2': ret.code = kVK_ANSI_Keypad2;  break;
		case '\x80'+'3': ret.code = kVK_ANSI_Keypad3;  break;
		case '\x80'+'4': ret.code = kVK_ANSI_Keypad4;  break;
		case '\x80'+'5': ret.code = kVK_ANSI_Keypad5;  break;
		case '\x80'+'6': ret.code = kVK_ANSI_Keypad6;  break;
		case '\x80'+'7': ret.code = kVK_ANSI_Keypad7;  break;
		case '\x80'+'8': ret.code = kVK_ANSI_Keypad8;  break;
		case '\x80'+'9': ret.code = kVK_ANSI_Keypad9;  break;
	}
	return ret;
}


struct nameToKeyCode {
	char name[16];
	uint32_t code;
} nameToKeyCode[] = {
	//12345678901234", kVK_...  <- see, all these strings are < 15 chars! :3
	{"a",              kVK_ANSI_A},
	{"s",              kVK_ANSI_S},
	{"d",              kVK_ANSI_D},
	{"f",              kVK_ANSI_F},
	{"h",              kVK_ANSI_H},
	{"g",              kVK_ANSI_G},
	{"z",              kVK_ANSI_Z},
	{"x",              kVK_ANSI_X},
	{"c",              kVK_ANSI_C},
	{"v",              kVK_ANSI_V},
	{"b",              kVK_ANSI_B},
	{"q",              kVK_ANSI_Q},
	{"w",              kVK_ANSI_W},
	{"e",              kVK_ANSI_E},
	{"r",              kVK_ANSI_R},
	{"y",              kVK_ANSI_Y},
	{"t",              kVK_ANSI_T},
	{"n1",             kVK_ANSI_1},
	{"n2",             kVK_ANSI_2},
	{"n3",             kVK_ANSI_3},
	{"n4",             kVK_ANSI_4},
	{"n6",             kVK_ANSI_6},
	{"n5",             kVK_ANSI_5},
	{"equal",          kVK_ANSI_Equal}, {"=", kVK_ANSI_Equal},
	{"n9",             kVK_ANSI_9},
	{"n7",             kVK_ANSI_7},
	{"minus",          kVK_ANSI_Minus}, {"-", kVK_ANSI_Minus},
	{"n8",             kVK_ANSI_8},
	{"n0",             kVK_ANSI_0},
	{"rightbracket",   kVK_ANSI_RightBracket}, {"]", kVK_ANSI_RightBracket},
	{"o",              kVK_ANSI_O},
	{"u",              kVK_ANSI_U},
	{"leftbracket",    kVK_ANSI_LeftBracket},  {"[", kVK_ANSI_LeftBracket},
	{"i",              kVK_ANSI_I},
	{"p",              kVK_ANSI_P},
	{"l",              kVK_ANSI_L},
	{"j",              kVK_ANSI_J},
	{"quote",          kVK_ANSI_Quote}, {"'", kVK_ANSI_Quote},
	{"k",              kVK_ANSI_K},
	{"semicolon",      kVK_ANSI_Semicolon}, {";", kVK_ANSI_Semicolon},
	{"backslash",      kVK_ANSI_Backslash}, {"\\", kVK_ANSI_Backslash},
	{"comma",          kVK_ANSI_Comma},     {",", kVK_ANSI_Comma},
	{"slash",          kVK_ANSI_Slash},     {"/", kVK_ANSI_Slash},
	{"n",              kVK_ANSI_N},
	{"m",              kVK_ANSI_M},
	{"period",         kVK_ANSI_Period}, {".", kVK_ANSI_Period},
	{"grave",          kVK_ANSI_Grave},  {"`", kVK_ANSI_Grave},
	{"keypaddecimal",  kVK_ANSI_KeypadDecimal},  {"k.", kVK_ANSI_KeypadDecimal},
	{"keypadmultiply", kVK_ANSI_KeypadMultiply}, {"k*", kVK_ANSI_KeypadMultiply},
	{"keypadplus",     kVK_ANSI_KeypadPlus},     {"k+", kVK_ANSI_KeypadPlus},
	{"keypadclear",    kVK_ANSI_KeypadClear},    {"kc", kVK_ANSI_KeypadClear},
	{"keypaddivide",   kVK_ANSI_KeypadDivide},   {"k/", kVK_ANSI_KeypadDivide},
	{"keypadenter",    kVK_ANSI_KeypadEnter},    {"ke", kVK_ANSI_KeypadEnter},
	{"keypadminus",    kVK_ANSI_KeypadMinus},    {"k-", kVK_ANSI_KeypadMinus},
	{"keypadequals",   kVK_ANSI_KeypadEquals},   {"k=", kVK_ANSI_KeypadEquals},
	{"keypad0",        kVK_ANSI_Keypad0}, {"k0", kVK_ANSI_Keypad0},
	{"keypad1",        kVK_ANSI_Keypad1}, {"k1", kVK_ANSI_Keypad1},
	{"keypad2",        kVK_ANSI_Keypad2}, {"k2", kVK_ANSI_Keypad2},
	{"keypad3",        kVK_ANSI_Keypad3}, {"k3", kVK_ANSI_Keypad3},
	{"keypad4",        kVK_ANSI_Keypad4}, {"k4", kVK_ANSI_Keypad4},
	{"keypad5",        kVK_ANSI_Keypad5}, {"k5", kVK_ANSI_Keypad5},
	{"keypad6",        kVK_ANSI_Keypad6}, {"k6", kVK_ANSI_Keypad6},
	{"keypad7",        kVK_ANSI_Keypad7}, {"k7", kVK_ANSI_Keypad7},
	{"keypad8",        kVK_ANSI_Keypad8}, {"k8", kVK_ANSI_Keypad8},
	{"keypad9",        kVK_ANSI_Keypad9}, {"k9", kVK_ANSI_Keypad9},
	{"return",         kVK_Return},
	{"tab",            kVK_Tab},
	{"space",          kVK_Space},   {" ", kVK_Space},
	{"delete",         kVK_Delete},  {"backspace", kVK_Delete}, {"del", kVK_Delete},
	{"escape",         kVK_Escape},  {"esc", kVK_Escape},
	{"command",        kVK_Command}, {"cmd", kVK_Command},
	{"shift",          kVK_Shift},
	{"capslock",       kVK_CapsLock},
	{"option",         kVK_Option},  {"alt", kVK_Option},
	{"control",        kVK_Control}, {"ctrl", kVK_Control}, {"^", kVK_Control},
	{"rightshift",     kVK_RightShift},   {"rshift", kVK_RightShift},
	{"rightoption",    kVK_RightOption},  {"ralt",   kVK_RightOption},
	{"rightcontrol",   kVK_RightControl}, {"rctrl",  kVK_RightControl}, {"r^", kVK_Control},
	{"function",       kVK_Function},     {"fn", kVK_Function},
	{"f17",            kVK_F17},
	{"volumeup",       kVK_VolumeUp},
	{"volumedown",     kVK_VolumeDown},
	{"mute",           kVK_Mute},
	{"f18",            kVK_F18},
	{"f19",            kVK_F19},
	{"f20",            kVK_F20},
	{"f5",             kVK_F5},
	{"f6",             kVK_F6},
	{"f7",             kVK_F7},
	{"f3",             kVK_F3},
	{"f8",             kVK_F8},
	{"f9",             kVK_F9},
	{"f11",            kVK_F11},
	{"f13",            kVK_F13},
	{"f16",            kVK_F16},
	{"f14",            kVK_F14},
	{"f10",            kVK_F10},
	{"f12",            kVK_F12},
	{"f15",            kVK_F15},
	{"help",           kVK_Help},
	{"home",           kVK_Home},
	{"pageup",         kVK_PageUp},        {"pgup", kVK_PageUp},
	{"forwarddelete",  kVK_ForwardDelete}, {"fdel", kVK_ForwardDelete},
	{"f4",             kVK_F4},
	{"end",            kVK_End},
	{"f2",             kVK_F2},
	{"pagedown",       kVK_PageDown},   {"pgdown", kVK_PageDown},
	{"f1",             kVK_F1},
	{"leftarrow",      kVK_LeftArrow},  {"left",  kVK_LeftArrow},
	{"rightarrow",     kVK_RightArrow}, {"right", kVK_RightArrow},
	{"downarrow",      kVK_DownArrow},  {"down",  kVK_DownArrow},
	{"uparrow",        kVK_UpArrow},    {"up",    kVK_UpArrow},
};

int comparNameToKeyCode(const void *a, const void *b) {
	return strcasecmp((const char *)a, (const char *)b);
}

static void __attribute__((constructor))initNameToKeyCode() {
	qsort(nameToKeyCode, sizeof(nameToKeyCode)/sizeof(nameToKeyCode[0]), sizeof(nameToKeyCode[0]), &comparNameToKeyCode);
}

uint32_t keyCodeForName(char *name) {
	struct nameToKeyCode *match = bsearch(name, nameToKeyCode, sizeof(nameToKeyCode)/sizeof(nameToKeyCode[0]), sizeof(nameToKeyCode[0]), &comparNameToKeyCode);
	if(match) return match->code;
	return -1;
}

static CGEventRef shiftDnEvent = NULL;
static CGEventRef shiftUpEvent   = NULL;

static void __attribute__((constructor)) initShiftEvents() {
	 shiftDnEvent = CGEventCreateKeyboardEvent(NULL, kVK_Shift, true);
	 shiftUpEvent = CGEventCreateKeyboardEvent(NULL, kVK_Shift, false);
}

ProcessSerialNumber eventpsn = {0, 0};

void postEvent(CGEventRef event) {
	if(eventpsn.highLongOfPSN || eventpsn.lowLongOfPSN) {
		CGEventPostToPSN(&eventpsn, event);
	} else {
		CGEventPost(kCGSessionEventTap, event);
	}
}

void performDown(struct KeyCode kc) {
	if(kc.flags & KeyCodeBeep) putchar('\a');
	if(kc.code == 0xFFFF) return;
	CGEventRef event = CGEventCreateKeyboardEvent(NULL, kc.code, false);
	if(kc.flags & KeyCodeShift) {
		postEvent(shiftDnEvent);
		usleep(keylag/2);
	}
	postEvent(event);
	CFRelease(event);
}

void performUp(struct KeyCode kc) {
	if(kc.code == 0xFFFF) return;
	CGEventRef event = CGEventCreateKeyboardEvent(NULL, kc.code, true);
	postEvent(event);
	CFRelease(event);
	if(kc.flags & KeyCodeShift) {
		postEvent(shiftUpEvent);
	}
}

enum CommandMode {
	COMMAND_NORMAL,
	COMMAND_DOWN,
	COMMAND_UP,
};

void commandC(char *cmd, enum CommandMode mode) {
	char *token;
	while(1) {
		token = strsep(&cmd, ",");
		if(token && token[0]) {
			struct KeyCode kc = {0,0};
			kc.code = isdigit(token[0]) ? strtol(token, NULL, 0) : keyCodeForName(token);
			switch(mode) {
				case COMMAND_NORMAL: performDown(kc); usleep(keylag); performUp(kc); usleep(keylag); break;
				case COMMAND_DOWN:   performDown(kc); usleep(keylag); break;
				case COMMAND_UP:     performUp(kc); usleep(keylag); break;
			}
		}
		usleep(waitkp);
		if(!cmd) break;
		cmd[-1] = ',';
	}
}

void commandE(char *cmd, enum CommandMode mode) {
	for(char *c = cmd; *c != 0; c++) {
		struct KeyCode kc = keyCodeForAscii(*c);
		switch(mode) {
			case COMMAND_NORMAL: performDown(kc); usleep(keylag); performUp(kc); usleep(keylag); break;
			case COMMAND_DOWN:   performDown(kc); usleep(keylag); break;
			case COMMAND_UP:     performUp(kc); usleep(keylag); break;
		}
		usleep(waitkp);
	}
}

static void error_unrecognized_flag(char *flag) {
	fprintf(stderr, "Error: Unrecognized flag %s\n", flag);
	usage(-1);
}

static void error_expected_arg(char *flag) {
	fprintf(stderr, "Error: Expected an argument to flag %s\n", flag);
	usage(-1);
}

int main(int argc, char *argv[]) {
	if(argc <= 1) usage(64);
	
	for(int i = 1; i < argc; i++) {
		if(argv[i][0] != '-')
			fprintf(stderr, "Error: Expected '-' to start arg in %s\n", argv[i]);
			//usage(-1);
		switch(argv[i][1]) {
			default: error_unrecognized_flag(argv[i]);
			case 'e':
				if(argv[i][2] != 0) error_unrecognized_flag(argv[i]);
				if(++i >= argc) error_expected_arg(argv[i-1]);
				commandE(argv[i], COMMAND_NORMAL);
				break;
			case 'c':
				if(argv[i][2] != 0) error_unrecognized_flag(argv[i]);
				if(++i >= argc) error_expected_arg(argv[i-1]);
				commandC(argv[i], COMMAND_NORMAL);
				break;
			case 's': 
				if(argv[i][2] != 0) error_unrecognized_flag(argv[i]);
				if(++i >= argc) error_expected_arg(argv[i-1]);
				fsleep(strtod(argv[i], NULL));
				break;
			case 'w':
				if(argv[i][2] != 0) error_unrecognized_flag(argv[i]);
				if(++i >= argc) error_expected_arg(argv[i-1]);
				waitkp = 1000.0*strtod(argv[i], NULL);
				break;
			case 'l':
				if(argv[i][2] != 0) error_unrecognized_flag(argv[i]);
				if(++i >= argc) error_expected_arg(argv[i-1]);
				keylag = 1000.0*strtod(argv[i], NULL);
				break;
			case 'p': {
				if(argv[i][2] != 0) error_unrecognized_flag(argv[i]);
				if(++i >= argc) error_expected_arg(argv[i-1]);
				long pid = strtol(argv[i], NULL, 0);
				if(pid == 0) {
					eventpsn.highLongOfPSN = 0;
					eventpsn.lowLongOfPSN = 0;
				} else if(GetProcessForPID(pid, &eventpsn)) {
					fprintf(stderr, "couldn't get process for pid %ld\n", pid);
					exit(-1);
				}
				break;
			}
			case 'm':
				if(argv[i][2] != 0) error_unrecognized_flag(argv[i]);
				if(++i >= argc) error_expected_arg(argv[i-1]);
				fprintf(stderr, "-m not supported yet, sorry\n");
				exit(1);
				break;
			case 'd': {
				switch(argv[i][2]) {
					default: error_unrecognized_flag(argv[i]);
					case 'e':
						if(argv[i][3] != 0) error_unrecognized_flag(argv[i]);
						if(++i >= argc) error_expected_arg(argv[i-1]);
						commandE(argv[i], COMMAND_DOWN);
						break;
					case 'c': 
						if(argv[i][3] != 0) error_unrecognized_flag(argv[i]);
						if(++i >= argc) error_expected_arg(argv[i-1]);
						commandC(argv[i], COMMAND_DOWN);
						break;
				}
				break;
			}
			case 'u': {
				switch(argv[i][2]) {
					default: error_unrecognized_flag(argv[i]);
					case 'e':
						if(argv[i][3] != 0) error_unrecognized_flag(argv[i]);
						if(++i >= argc) error_expected_arg(argv[i-1]);
						commandE(argv[i], COMMAND_UP);
						break;
					case 'c':
						if(argv[i][3] != 0) error_unrecognized_flag(argv[i]);
						if(++i >= argc) error_expected_arg(argv[i-1]);
						commandC(argv[i], COMMAND_UP); break;
					case 'u': {
						if(argv[i][3] != 0) error_unrecognized_flag(argv[i]);
						fprintf(stderr, "-uu not supported yet, sorry\n");
						exit(1);
					}
					case 'd': {
						if(argv[i][3] != 0) error_unrecognized_flag(argv[i]);
						fprintf(stderr, "-ud not supported yet, sorry\n");
						exit(1);
					}
				}
				break;
			}
		}
	}
	
	return 0;
}
