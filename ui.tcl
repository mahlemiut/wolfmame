#!/usr/bin/env wish

# set up window
wm title . "Basic MARP GUI"
wm geometry . 600x600

# GUI elements
frame .border -relief groove -borderwidth 5 -padx 5 -pady 5
frame .listframe -relief groove -borderwidth 5 -padx 5 -pady 5
label .txt1 -text "Select game:"
listbox .gamelist -yscrollcommand ".scroll set" -height 20 -font {Helvetica 12}
label .txt2 -text "Additional parameters:"
entry .param
label .txt3 -text "INP filename:"
entry .inppath
button .record -text "Record INP"
button .play -text "Play INP"
scrollbar .scroll -orient v -command ".gamelist yview"
button .browse -text "Browse..."
button .run -text "Run Machine"

# Add a search entry and button
label .search_label -text "Search:"
entry .search_entry
button .search_button -text "Search" -command click_search

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
    global filtered_indices
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
    set filtered [lindex $filtered_indices $index]
    set command $cmdfilename
    append command " "
    append command [lindex $gamelistshortnames $filtered]
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
    global filtered_indices
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
    set filtered [lindex $filtered_indices $index]
    set command $cmdfilename
    append command " "
    append command [lindex $gamelistshortnames $filtered]
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

proc click_run {} {
    global cmdfilename
    global gamelistshortnames
    global filtered_indices
    set sel [.gamelist curselection]
    if { $sel == "" } {
        tk_messageBox -message "Please select a game from the list"
        return
    }
    set index [lindex $sel 0]
    set filtered [lindex $filtered_indices $index]
    set command $cmdfilename
    append command " "
    append command [lindex $gamelistshortnames $filtered]
    append command " "
    append command [.param get]
    tk_messageBox -message $command
    set runcmd [open $command r]
    set output [read $runcmd]
    close $runcmd
    tk_messageBox -title "Console output" -message $output
}

# Procedure to handle the search
proc click_search {} {
    global gamelistitems gamelistshortnames filtered_indices
    set search_term [.search_entry get]
    .gamelist delete 0 end ;# Clear the listbox
    set filtered_indices [list] ;# Reset the filtered indices

    # Filter and repopulate the listbox
    for {set index 0} {$index < [llength $gamelistitems]} {incr index} {
        set name [lindex $gamelistshortnames $index]
        set description [lindex $gamelistitems $index]
        if {[string match -nocase *$search_term* $name]} {
            .gamelist insert end "$name - $description"
            lappend filtered_indices $index ;# Store the original index
        }
    }
}

# figure out if we're using a 32-bit or 64-bit version of MAME
set cmdfilename "|./mame"

if { [catch {exec ./mame -help}] } {
    set cmdfilename "|./mame64"
    if { [catch {exec ./mame64 -help}] } {
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
set filtered_indices [list] ;# To track filtered indices
set gamelistshortnames [list]

# Parse MAME's -listxml output
package require tdom
set cmdline $cmdfilename
append cmdline " -listxml"
set cmd [open $cmdline r]
set xml_data [read $cmd]
close $cmd

# Parse XML
set doc [dom parse $xml_data]

# Select all runnable machines
set machines [$doc selectNodes {//machine[@runnable='yes']}]

# Populate the listbox
set filtered_indices [list] ;# Initialize the filtered indices list
foreach machine $machines {
    set name [$machine getAttribute name]
    set description [$machine selectNodes "description"]
    set description_text [$description asText]
    lappend gamelistshortnames $name
    lappend gamelistitems $description_text
    lappend filtered_indices [expr {[llength $gamelistshortnames] - 1}] ;# Correct the index
    .gamelist insert end "$name - $description_text"
}

# Update the title bar with the count of machines
set machine_count [llength $gamelistshortnames]
wm title . "Basic MARP GUI - $machine_count Machines"

# GUI management
grid columnconfigure . {0 1 2 3 4} -weight 1
grid columnconfigure . 5 -weight 0
grid rowconfigure . {0 3 4 5 6 7 8} -weight 0
grid rowconfigure . 2 -weight 1
grid .search_label -row 0 -column 0
grid .search_entry -row 0 -column 1 -columnspan 2 -sticky ew
grid .search_button -row 0 -column 3
grid .txt1 -row 1 -column 0 -columnspan 6
grid .gamelist -row 2 -column 0 -columnspan 5 -sticky nsew
grid .scroll -row 2 -column 5 -sticky ns
grid .txt3 -row 3 -column 0 -columnspan 6 -sticky ew
grid .inppath -row 4 -column 0 -columnspan 6 -sticky ew
grid .txt2 -row 5 -column 0 -columnspan 6 -sticky ew
grid .param -row 6 -column 0 -columnspan 6 -sticky ew
grid .record -row 7 -column 1 -sticky ew
grid .play -row 7 -column 2 -sticky ew
grid .run -row 7 -column 3 -sticky ew

# event bindings
#bind .browse <ButtonPress-1> { click_browse }
bind .record <ButtonPress-1> { click_record }
bind .play <ButtonPress-1> { click_playback }
bind .run <ButtonPress-1> { click_run }
