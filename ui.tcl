#!/bin/sh
# the next line restarts using wish \
exec wish "$0" "$@"

# set up window
wm title . "Basic MARP GUI"
wm geometry . 600x600

# GUI elements
frame .border -relief groove -borderwidth 5 -padx 5 -pady 5
frame .listframe -relief groove -borderwidth 5 -padx 5 -pady 5
label .txt1 -text "Select game:"
listbox .gamelist -yscrollcommand ".scroll set" -height 800 -font courier
label .txt2 -text "Additional parameters:"
entry .param
label .txt3 -text "INP filename:"
entry .inppath
button .record -text "Record INP"
button .play -text "Play INP"
scrollbar .scroll -orient v -command ".gamelist yview"
button .browse -text "Browse..."

# event procedures
proc click_browse {} {
set ftypes {
	{{MAME input logs} {*.inp}}
	{{All files} {*}}
}
set filename [tk_getOpenFile -filetypes $ftypes]
.inppath delete 0 end ;# clear entry box
.inppath insert 0 $filename
}

proc click_record {} {
global cmdfilename
global gamelistshortnames
global nullfile
set sel [.gamelist curselection]
if { $sel == "" } {
	tk_messageBox -message "Please select a game from the list"
	return
}
if { [.inppath get] == "" } {
	tk_messageBox -message "Please enter an INP filename to record to"
	return
}
if { [string match "*.inp" [.inppath get]] == 0} {
        tk_messageBox -message "INP filenames require the .inp extension"
        return
}
set index [lindex $sel 0]
set command $cmdfilename
append command " "
append command [lindex $gamelistshortnames $index]
append command " -record "
append command [.inppath get]
append command " -nvram_directory "
append command $nullfile
append command " "
append command [.param get]
tk_messageBox -message $command
set reccmd [open $command r]
set output [read $reccmd]
close $reccmd
tk_messageBox -title "Console output" -message $output
}

proc click_playback {} {
global cmdfilename
global gamelistshortnames
global nullfile
set sel [.gamelist curselection]
if { $sel == "" } {
        tk_messageBox -message "Please select a game from the list"
        return
}
if { [.inppath get] == "" } {
        tk_messageBox -message "Please enter an INP filename to playback"
        return
}
if { [string match "*.inp" [.inppath get]] == 0} {
	tk_messageBox -message "INP filenames require the .inp extension"
	return
}
set index [lindex $sel 0]
set command $cmdfilename
append command " "
append command [lindex $gamelistshortnames $index]
append command " -playback "
append command [.inppath get]
append command " -nvram_directory "
append command $nullfile
append command " "
append command [.param get]
tk_messageBox -message $command
set playcmd [open $command r]
set output [read $playcmd]
close $playcmd
tk_messageBox -title "Console output" -message $output
}

# figure out if we're using a 32-bit or 64-bit version of MAME
set cmdfilename "|./mamearcade"

if { [catch {exec ./mamearcade -help}] } {
	set cmdfilename "|./mamearcade64"
	if { [catch {exec ./mamearcade64 -help}] } {
		set cmdfilename "none"
		tk_messageBox -title "Error" -type ok -message "Cannot find a MAME executable."
		exit 1
	}
}

# figure out the platform we are running on
global tcl_platform
switch -glob -- [lindex $tcl_platform(os) 0] {
	Win* {
		set nullfile "NUL"  ;# Windows
	}
	default {
		set nullfile "/dev/null"  ;# everything else (except maybe old Macs)
	}
}

# populate listbox
set gamelistitems [list]
set gamelistshortnames [list]
set cmdline $cmdfilename
append cmdline " -listfull"
set cmd [open $cmdline r]
# TODO: handle ini parsing errors, which will screw this up
gets $cmd ;# ignore first line, it contains the headers
while {[gets $cmd line] >= 0} {
	set gamedata [split $line "\t\""]
	set str [string trim [lindex $gamedata 0] " "]
	lappend gamelistshortnames $str
	lappend gamelistitems [lindex $gamedata 1]
	set listtext [lindex $gamedata 0]
	append listtext [lindex $gamedata 1]
	.gamelist insert end $listtext
}
close $cmd


# GUI management
grid columnconfigure . {0 1 2 3 4} -weight 1
grid columnconfigure . 5 -weight 0
grid rowconfigure . {0 2 3 4 5 6} -weight 0
grid rowconfigure . 1 -weight 3
grid .txt1 -row 0 -column 0 -columnspan 6
grid .gamelist -row 1 -column 0 -columnspan 5 -sticky ew
grid .scroll -row 1 -column 5 -sticky nse
grid .txt3 -row 2 -column 0 -columnspan 6
grid .inppath -row 3 -column 0 -columnspan 6 -sticky ew
#grid .browse -row 3 -column 4 -columnspan 2 -sticky nsew
grid .txt2 -row 4 -column 0 -columnspan 6
grid .param -row 5 -column 0 -columnspan 6 -sticky ew
grid .record -row 6 -column 1
grid .play -row 6 -column 3

# event bindings
#bind .browse <ButtonPress-1> { click_browse }
bind .record <ButtonPress-1> { click_record }
bind .play <ButtonPress-1> { click_playback }
