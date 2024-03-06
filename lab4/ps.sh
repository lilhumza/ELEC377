#!/bin/bash

# ELEC377 - Operating System
# Lab 4 - Shell Scripting, ps.sh
# Program Description:
# This script lists all running processes in the VM, with options to show additional details.

# Temporary file for sorted output
tmpFile="/tmp/tmpPs$$"

# Initializing boolean variables for flags inputted by user
showRSS=false
showComm=false
showCmd=false
showGroup=false

while (($# > 0))
do
    if [[ $1 == "-rss" ]];then
        showRSS=true
    elif [[ $1 == "-comm" ]];then
        showComm=true
    elif [[ $1 == "-command" ]];then
        showCmd=true
    elif [[ $1 == "-group" ]];then
        showGroup=true
    else
        echo "Error: Unrecognized Flag Provided ${1}"
        exit 33
    fi

    shift
done

if [ "$showComm" = true ] && [ "$showCmd" = true ]; then
    echo "Error: Both -command -comm flags detected"
    exit 66
# else
#     # Print Status of All Flags
#     echo "showRSS = $showRSS"
#     echo "showComm = $showComm"
#     echo "showCmd = $showCmd"
#     echo "showGroup = $showGroup"
fi

# Processing /proc directories
for p in /proc/[0-9]*; do
    if [[ -d "$p" ]]; then
        # Extract process ID
        pid=$(grep '^Pid:' "$p/status" | sed -n -e 's/Pid:[[:space:]]*\([0-9]\+\)/\1/p')

        # Extract short command name
        name=$(grep '^Name:' "$p/status" | sed -n -e '/^Name:/ s/Name:[[:space:]]*\(.*\)/\1/p')

        # Extract user ID
        uid=$(grep '^Uid:' "$p/status" | sed -n -e '/^Uid:/ s/Uid:[[:space:]]*\([0-9]\+\).*/\1/p')

        # Extract group ID
        gid=$(grep '^Gid:' "$p/status" | sed -n -e '/^Gid:/ s/Gid:[[:space:]]*\([0-9]\+\).*/\1/p')

        # Extract resident set size
        rss=$(grep '^VmRSS:' "$p/status" | sed -n -e '/^VmRSS:/ s/VmRSS:[[:space:]]*\([0-9]\+\).*/\1/p')
        
        # Command line
        if [ "$showCmd" = true ];then
            # Remove \0 NULL with blank spaces
            cmdline=$(tr -d '\0' < "$p/cmdline" | tr '\0' ' ')
            # Check if result of tr is empty, otherwise default
            [ -z "$cmdline" ] && cmdline=$name
        fi

        # Mapping numeric user ID to symbolic username - with defaulting case back to numeric
        userName=$(awk -F ':' -v uid="$uid" '$3 == uid {print $1; found=1} END {if (!found) print uid}' /etc/passwd)

        # Mapping numeric group ID to symbolic group name
        groupName=$(awk -F ':' -v gid="$gid" '$3 == gid {print $1}' /etc/group)

        # Constructing output of the various flags and baseline of PID UID
        printf "%-6s %-32s" "$pid" "$userName" >> /tmp/tmpPs$$.txt

        if [ "$showRSS" = true ]; then
            printf "%-32s" "$rss" >> /tmp/tmpPs$$.txt
        fi

        if [ "$showComm" = true ]; then
            printf "%-32s" "$name" >> /tmp/tmpPs$$.txt
        fi

        if [ "$showCmd" = true ]; then
            printf "%-32s" "$cmdline" >> /tmp/tmpPs$$.txt
        fi

        if [ "$showGroup" = true ]; then
            printf "%-32s" "$groupName" >> /tmp/tmpPs$$.txt
        fi
    fi
    printf "\n" >> /tmp/tmpPs$$.txt 
done

# Formatting Headers and respective variable fields
printf "%-6s %-32s" "PID" "USER"

if [ "$showRSS" = true ]; then
    printf "%-32s" "RSS"
fi

if [ "$showComm" = true ]; then
    printf "%-32s" "COMM"
fi

if [ "$showCmd" = true ]; then
    printf "%-32s" "CMD"
fi

if [ "$showGroup" = true ]; then
    printf "%-32s" "GROUP"
fi

printf "\n"

# Sort and display the output
sort -n -k 1 /tmp/tmpPs$$.txt

# Remove temporary file
rm /tmp/tmpPs$$.txt

exit 100
