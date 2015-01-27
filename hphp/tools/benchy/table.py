#!/usr/bin/env python
"""Encapsulated table class for pretty printing tabular data.

"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import json
import sys
import re

ANSI_ESCAPE = re.compile(r'\033[^m]*m')

def _print_horizontal_line(max_widths):
    """Prints a horizontal line across all columns in the table.

    """
    for width in max_widths:
        dashes = '-' * width
        sys.stdout.write("%s---" % dashes)
    sys.stdout.write('-\n')

def _print_entry_centered(entry, width):
    """Prints a table entry with equal space on each side.

    """
    total_padding = width - _len_sans_ansi(entry)
    front_padding = ' ' * (total_padding // 2)
    back_padding = ' ' * (total_padding - len(front_padding))
    sys.stdout.write("%s%s%s" % (front_padding, entry, back_padding))

def _print_entry_left(entry, width, filler=' '):
    """Prints a table entry left justified within its cell.

    """
    total_padding = width - _len_sans_ansi(entry)
    back_padding = filler * total_padding
    sys.stdout.write("%s%s" % (entry, back_padding))

def _len_sans_ansi(text):
    """Computes the length of a string that might contain ANSI codes.

    """
    return len(ANSI_ESCAPE.sub('', text))

class Table(object):
    """Encapsulation around a set of column headers and a series of rows of
    data which gracefully handles pretty printing the data it contains in a
    variety of tabular formats.

    """
    def __init__(self, headers):
        self._headers = headers
        self._row_length = len(headers)
        self._rows = []

    def add_row(self, row):
        """Append a new row of data.

        """
        if len(row) != self._row_length:
            raise RuntimeError("Invalid row length")
        self._rows.append(row)

    def _find_max_column_widths(self, headers, rows):
        """Finds the maximum width of each column based on its contents.

        """
        # Find the max width for each column
        max_widths = [_len_sans_ansi(x) for x in headers]
        for row in rows:
            for i in range(self._row_length):
                width = _len_sans_ansi(str(row[i]))
                if width > max_widths[i]:
                    max_widths[i] = width
        return max_widths

    def dump(self, out_format):
        """Dumps the table in the specified out_format.  Valid values are
        'terminal', 'remarkup', and 'json'.

        """
        if out_format == 'terminal':
            self.dump_to_terminal()
        elif out_format == 'remarkup':
            self.dump_to_remarkup()
        elif out_format == 'json':
            self.dump_to_json()
        else:
            raise RuntimeError("Unknown output format: %s" % out_format)

    def dump_to_json(self):
        """Print the table in JSON format. The printed value will be an array
        of JSON objects corresponding to each row.

        """
        rows = [dict((self._headers[i], row[i]) for i in range(len(row)))
            for row in self._rows]
        json.dump(rows, sys.stdout)

    def dump_to_remarkup(self):
        """Pretty print the table headers and rows in remarkup format.

        """
        headers, rows = self._headers, self._rows
        max_widths = self._find_max_column_widths(headers, rows)

        # Print the headers.
        sys.stdout.write("|")
        for i in range(len(headers)):
            sys.stdout.write(' ')
            _print_entry_centered(headers[i], max_widths[i])
            sys.stdout.write(" |")
        sys.stdout.write('\n')

        sys.stdout.write("|")
        for i in range(len(headers)):
            sys.stdout.write("-")
            _print_entry_left('', max_widths[i], filler='-')
            sys.stdout.write("-|")
        sys.stdout.write('\n')

        # Print each row
        for i in range(len(self._rows)):
            row = self._rows[i]
            sys.stdout.write("|")
            for i in range(len(row)):
                sys.stdout.write(" ")
                _print_entry_left(str(row[i]), max_widths[i])
                sys.stdout.write(" |")
            sys.stdout.write('\n')

    def dump_to_terminal(self):
        """Pretty prints the table headers and rows in a terminal friendly
        format.

        """
        headers, rows = self._headers, self._rows
        max_widths = self._find_max_column_widths(headers, rows)

        # Print the headers.
        _print_horizontal_line(max_widths)

        sys.stdout.write("|")
        for i in range(len(headers)):
            sys.stdout.write(" ")
            _print_entry_centered(headers[i], max_widths[i])
            sys.stdout.write(" |")
        sys.stdout.write('\n')

        _print_horizontal_line(max_widths)

        # Print each row.
        for row in rows:
            sys.stdout.write("|")
            for i in range(len(row)):
                sys.stdout.write(" ")
                _print_entry_left(str(row[i]), max_widths[i])
                sys.stdout.write(" |")
            sys.stdout.write('\n')

        _print_horizontal_line(max_widths)
