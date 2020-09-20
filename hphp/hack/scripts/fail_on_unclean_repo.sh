#!/bin/bash
echo "List of modified files:"
CHANGED_FILES="$(hg st)"
echo "$CHANGED_FILES"
if [[ -z "$CHANGED_FILES" ]]
then
    echo "No files changed!"
else
    echo "ERROR: Files have changed! Please run \"$1\" and amend generated files into diff."
    exit 1
fi
