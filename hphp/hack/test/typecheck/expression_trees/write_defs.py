#!/usr/bin/env python3

import os


def main():
    # Load the definitions from expr_tree_defs.php
    def_file = "expr_tree_defs.php"
    def_file_handle = open(def_file, "r")
    def_file_read = def_file_handle.readlines()
    defs = ""
    copy = False
    for line in def_file_read:
        if line.find("BEGIN DEFS") != -1:
            copy = True
        if copy:
            defs += line
        if line.find("END DEFS") != -1:
            copy = False
    def_file_handle.close()

    # Go through the remaining files in expression_trees/
    curr_dir = os.getcwd()
    for entry in os.listdir(curr_dir):
        if (entry.endswith(".php")) and entry != def_file:
            # Rewrite the file
            entry_handle = open(entry, "r")
            entry_read = entry_handle.readlines()
            new_entry = ""
            copy = True
            for line in entry_read:
                if line.find("BEGIN DEFS") != -1:
                    copy = False
                    new_entry += defs
                if copy:
                    new_entry += line
                if line.find("END DEFS") != -1:
                    copy = True
            entry_handle.close()
            entry_handle = open(entry, "w")
            entry_handle.write(new_entry)
            entry_handle.close()


if __name__ == "__main__":
    main()
