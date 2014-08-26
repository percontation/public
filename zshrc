# .zshrc
# Basic zsh setup
export HISTSIZE=100000
export SAVEHIST=5000
export HISTFILE=~/.zsh_history
setopt APPEND_HISTORY
setopt HIST_REDUCE_BLANKS
setopt HIST_IGNORE_DUPS
setopt HIST_FIND_NO_DUPS
setopt HIST_EXPIRE_DUPS_FIRST

bindkey -e
autoload -U compinit; compinit

PS1='%m:%1d%(?..[%?])%# '

# Env setup
export EDITOR='vim'
export PAGER='less -i'

# Colors!
if [ -z "$NOCOLOR" ]; then
	# It would be super nice if everything listened to the CLICOLOR
	# env variable... instead of just BSD ls (-.-)
	export CLICOLOR=1
	export GCC_COLORS=1
	alias egrep='egrep --color=auto'
	alias fgrep='fgrep --color=auto'
	alias grep='grep --color=auto'
	if ls --color=auto >/dev/null 2>&1
	 then alias ls='ls --color=auto'; fi
	if pacman --color=auto >/dev/null 2>&1
	 then alias pacman='pacman --color=auto'; fi
fi

# Custom aliasing
alias sudo='sudo ' # Trailing space means next arg is alias-expanded
alias cp='cp -i'
alias mv='mv -i'
alias ssh_unsafe="ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no"
alias scp_unsafe="scp -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no"
alias cd='filecd'
filecd() {
	# This function is sh-compatible; use it anywhere!
	'cd' "$@" && return
	
	# Because cd does take arguments sometimes.
	local ARGS TARG
	case "$1" in
		-L) ARGS="$1" TARG="$2";;
		-P) ARGS="$1" TARG="$2";;
		*)  ARGS="" TARG="$1";;
	esac

	test -d "$TARG" && return
	test -e "$TARG" || return

	test -n "$ARGS" && ARGS="$ARGS "
	local CMD="'cd' $ARGS'`dirname "$TARG"`'"
	echo Trying "$CMD" >&2
	eval "$CMD"
}


NOTAVM="`sh -c 'lscpu;true' 2>&1 | grep '^Hypervisor vendor:' -q || echo NOTAVM`"
if [ -n "$NOTAVM" ]; then
	alias poweroff='echo "You sure mate? `hostname` ain'"'"'t a VM." >&2 && false'
	alias shutdown='echo "You sure mate? `hostname` ain'"'"'t a VM." >&2 && false'
fi
if [ -n "$NOTAVM" ] && [ -z "$SSH_TTY" ]; then
	PS1='%B%m%b:%1d%(?..[%?])%# '
	alias reboot='echo "You sure mate? `hostname` is your computer!" >&2 && false'
fi

# Fancy zsh setup
autoload add-zsh-hook
add-zsh-hook preexec datecomment
datecomment() {
	# Comment entered lines with exec date.
	local STATUS=$?
	if [ $((`printf '%s' $1 | wc -l`)) -gt 0 ]; then
		# For multiline commands, don't be clever, things get really screwy.
		printf 'date> # %s\n' "`date`"
	else
		# Ideally, we'd just get the position of the cursor at the end of the
		# previous line. Since this seems to be impossible, we use shitty hacks
		# that are unlikely to work for fancy prompts.
		local LAST="`printf '%s%s' "$( (exit $STATUS); print -nP "$PS1")" "$1"`"
		local LEN=$((`printf '%s' "$LAST" | perl -pe 's/\e[^a-z]*[a-z]//gi' | wc -c`))
		local LINES=$(($LEN / `tput cols` + 1))
		printf '\e[%dA%s # %s\n' "$LINES" "$LAST" "`date '+%H:%M:%S'`"
	fi
}

# Convenience functions
help() {
	# Not the most portable or robust of things,
	# but it works for me right now :P
	man -P "$PAGER '+/^       $1'" zshbuiltins
}

urlencode() {
	printf '%s' "$1" | perl -pe 's/([^-_.A-Za-z0-9])/sprintf("%%%02X", ord($1))/seg'
}

urlpathencode() {
	printf '%s' "$1" | perl -pe 's/([^-_.A-Za-z0-9\/])/sprintf("%%%02X", ord($1))/seg'
}

####################

# OS or machine specific stuff

if [ "`uname`" = 'Darwin' ]; then
	alias make="make -j$((`sysctl -n hw.ncpu`/2+1))"
	
	opdflatex() {
		local OUTDIR="${TMPDIR:-/tmp}/$(basename "$1" ".tex")"
		mkdir -p "$OUTDIR"
		pdflatex -halt-on-error -output-directory="$OUTDIR" $@ && \
		open "$OUTDIR"/*.pdf
	}
	
	if [ "$TERM_PROGRAM" = 'Apple_Terminal' -o "$TERM_PROGRAM" = 'iTerm.app' ] && [ -z "$INSIDE_EMACS" ]; then
		update_terminal_cwd() {
			printf '\e]7;%s\a' "file://`urlencode "$HOSTNAME"``urlpathencode "$PWD"`"
		}
		autoload add-zsh-hook
		add-zsh-hook chpwd update_terminal_cwd
		update_terminal_cwd
	fi
fi

if [ "`uname`" = 'Linux' ]; then
	if which xsel >/dev/null 2>&1; then
		alias pbcopy='xsel --clipboard --input'
		alias pbpaste='xsel --clipboard --output'
	elif which xclip >/dev/null 2>&1; then
		alias pbcopy='xclip -selection clipboard'
		alias pbpaste='xclip -selection clipboard -o'
	fi
fi
