#!/usr/bin/env python
"""Pretty print statistics comparisons.

Parses files containing labeled lines of means and confidence intervals,
compares each of them against each other using confidence intervals to
determine which changes are significant, and pretty prints the results in a
table.

"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import argparse
import os
import re
from table import Table

LINE_REGEX = re.compile(r'(.+):\s*([0-9.]+)(?: \+-([0-9.]+))?')


class ResultFile(object):
    """A handle to a named file and its parsed contents.

    """
    def __init__(self, name, data):
        self.name = name
        self.data = data

    def short_name(self):
        """Returns the short name for this particular result file.

        The path is used as the name, so the short name is the basename.

        """
        return os.path.basename(self.name)


def read_input(filename, in_file):
    """Parses labeled averages and confidence intervals from the provided file.

    """
    data = {}
    for line in in_file:
        result = LINE_REGEX.match(line)
        if result is None:
            continue

        lhs = str(result.group(1))
        mean = str(result.group(2))
        conf_interv = result.group(3)

        try:
            mean = float(mean)
            if conf_interv is None:
                conf_interv = 0.0
            else:
                conf_interv = float(conf_interv)
        except ValueError:
            continue

        data[lhs] = (mean, conf_interv)
    return ResultFile(filename, data)


def transpose_result_data(result_files):
    """Reassociate data from multiple result files into single categories.

    We reorganize the data so that we can compare results on the same
    category across multiple result files.

    """
    categories = {}
    for result_file in result_files:
        for key, value in result_file.data.iteritems():
            if key not in categories:
                categories[key] = []
            categories[key].append((result_file.name, value))
    return categories


def confidence_intervals_overlap(old_score, old_ci, new_score, new_ci):
    """Returns true if the confidence intervals of the old and new scores
    overlap, false otherwise.

    """
    if old_score < new_score:
        old_score += old_ci
        new_score -= new_ci
        return old_score >= new_score
    else:
        old_score -= old_ci
        new_score += new_ci
        return old_score <= new_score


def percent_delta(old_score, new_score):
    """Calculates the percent change between two scores.

    """
    return float(new_score - old_score) / float(old_score)


def print_results(result_files, out_format):
    """Builds a table with the parsed results. Used when there is only one
    result file.

    """
    categories = transpose_result_data(result_files)
    columns = [result.short_name() for result in result_files]
    columns.insert(0, "Benchmark")
    table = Table(columns)

    geomean = None
    for key in categories:
        scores = [run[1] for run in categories[key]]
        entries = ["%.2f +- %.2f" % score for score in scores]
        entries.insert(0, key)
        if key == 'Geomean':
            geomean = entries
        else:
            table.add_row(entries)

    if geomean is not None:
        table.add_row(geomean)

    table.dump(out_format)


def red(text):
    """Returns a string with ANSI codes for red color and reset color wrapping
    the provided text.

    """
    return '\033[31m%s\033[39m' % text


def green(text):
    """Returns a string with ANSI codes for green color and reset color
    wrapping the provided text.

    """
    return '\033[32m%s\033[39m' % text


def bold(out_format, text):
    """Returns a string representing a bolded version of the given text
    depending on the output format.

    """
    if out_format == 'remarkup':
        return "**%s**" % text
    elif out_format == 'terminal':
        return "\033[1m%s\033[0m" % text
    elif out_format == 'json':
        return text
    else:
        raise RuntimeError("Unknown output format: %s" % out_format)


def faster(out_format, text):
    """Used to visually signify the given string represents a faster result.

    """
    if out_format == 'remarkup':
        return bold(out_format, text)
    elif out_format == 'terminal':
        return bold(out_format, green(text))
    elif out_format == 'json':
        return text
    else:
        raise RuntimeError("Unknown output format: %s" % out_format)


def slower(out_format, text):
    """Used to visually signify the given string represents a slower result.

    """
    if out_format == 'remarkup':
        return bold(out_format, text)
    elif out_format == 'terminal':
        return bold(out_format, red(text))
    elif out_format == 'json':
        return text
    else:
        raise RuntimeError("Unknown output format: %s" % out_format)


def is_slower(change, lower_is_better):
    """Returns true if change represents a slowdown with the current
    lower_is_better setting.

    """
    if lower_is_better:
        return change > 0.0
    return change < 0.0


def print_comparison_results(result_files, out_format, lower_is_better):
    """Builds a table of the various gathered results and prints it out.

    The table also includes an extra column for deltas and adds entries for
    significant changes between the last two provided files.

    """
    def entries_for_scores(key, scores):
        """Returns entries for the next row, prepending the category name and
        appending any significant changes.

        """
        old_score, old_ci = scores[-2]
        new_score, new_ci = scores[-1]
        entries = ["%.2f +- %.2f" % score for score in scores]
        entries.insert(0, key)
        if confidence_intervals_overlap(old_score, old_ci, new_score, new_ci):
            entries.append("")
        else:
            change = percent_delta(old_score, new_score)
            if is_slower(change, lower_is_better):
                change_str = "%.4f%% slower" % (change * 100.0)
                entries.append(slower(out_format, change_str))
            else:
                change_str = "+%.4f%% faster" % (change * 100.0)
                entries.append(faster(out_format, change_str))
        return entries

    categories = transpose_result_data(result_files)
    columns = [result.short_name() for result in result_files]
    columns.insert(0, "Benchmark")
    columns.append("Deltas")
    table = Table(columns)

    geomean = None
    for key in categories:
        entries = entries_for_scores(key, [run[1] for run in categories[key]])
        if key == 'Geomean':
            geomean = entries
            geomean[0] = bold(out_format, geomean[0])
        else:
            table.add_row(entries)

    if geomean is not None:
        table.add_row(geomean)

    table.dump(out_format)


def main():
    """Parses arguments and passes control off to the worker functions.

    """
    parser = argparse.ArgumentParser(description="Compare benchmark results "
                                                 "for significant changes.")
    parser.add_argument('--remarkup', action='store_const', const=True,
                        default=False, help='Spit out the results as Remarkup')
    parser.add_argument('--terminal', action='store_const', const=True,
                        default=False, help='Spit out the results in format '
                                            'that\'s nice for terminals')
    parser.add_argument('--json', action='store_const', const=True,
                        default=False, help='Spit out the results as JSON.')
    parser.add_argument('--lower-is-better', action='store_const', const=True,
                        default=False, help='Bases comparisons on the fact '
                                            'that lower is better.')
    parser.add_argument('file', metavar='FILE', nargs='+', type=str,
                        help='Files to parse for statistics.')
    args = parser.parse_args()

    out_format = None
    if args.terminal:
        out_format = 'terminal'
    elif args.remarkup:
        out_format = 'remarkup'
    elif args.json:
        out_format = 'json'
    else:
        out_format = 'terminal'

    lower_is_better = args.lower_is_better

    result_files = []
    for filename in args.file:
        with open(filename, 'r') as in_file:
            result_files.append(read_input(filename, in_file))
    if len(result_files) > 1:
        print_comparison_results(result_files, out_format, lower_is_better)
    else:
        print_results(result_files, out_format)


if __name__ == "__main__":
    main()
