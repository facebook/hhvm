#!/usr/bin/env python
"""Computes averages and confidence intervals of labeled data."""
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
from confidence_interval import mean_confidence_interval, arith_mean

import argparse
import math
import re
import sys

MEASUREMENT_REGEX = re.compile(r'(.+):\s*(.+)')


def parse_measurements(in_file):
    """Parses a set of labeled measurements and aggregates like-labeled
    measurements.

    """
    categories = {}
    for line in in_file:
        result = MEASUREMENT_REGEX.match(line)
        if result is None:
            continue

        lhs = str(result.group(1))
        rhs = str(result.group(2))

        try:
            rhs = float(rhs)
        except ValueError:
            continue

        if lhs not in categories:
            categories[lhs] = []
        categories[lhs].append(rhs)
    return categories


def find_widest_key(categories):
    """Returns width of widest key for formatting.

    """
    widest_key = 0
    for key in categories:
        if len(key) > widest_key:
            widest_key = len(key)
    return widest_key


def arithmetic_mean(samples):
    """Computes the arithmetic mean of a set of samples.

    """
    return float(sum(samples)) / float(len(samples))


def geometric_mean(samples):
    """Computes the geometric mean of a set of samples.

    """
    return math.exp(arithmetic_mean([math.log(x) for x in samples]))


# Select stripes across all categories and compute the geomean of these stripes
def compute_striped_geomeans(categories):
    """Pulls a sample from each category into a "stripe" and computes the
    geometric mean of the stripe.

    """
    geomeans = []
    i = 0
    while True:
        stripe = []
        for _, values in categories.iteritems():
            if i >= len(values):
                continue
            stripe.append(values[i])
        if len(stripe) == 0:
            break
        geomeans.append(geometric_mean(stripe))
        i += 1
    categories['Geomean'] = geomeans


def print_means_and_cis(categories, widest_key):
    """Prints the mean and confidence interval for each category.

    """
    for key, values in categories.iteritems():
        pad_width = widest_key - len(key)
        padding = " " * pad_width
        mean, interval = None, None
        if len(values) > 1:
            mean, interval = mean_confidence_interval(values)
            print("%s: %s%.2f +-%.2f" % (key, padding, mean, interval))
        else:
            mean = arith_mean(values)
            print("%s: %s%.2f" % (key, padding, mean))
            sys.stderr.write("Warning: too few samples to calculate confidence"
                             " interval for \"%s\"\n" % key)


def main():
    """Parses arguments and passes them to the main computation functions.

    """
    parser = argparse.ArgumentParser(description="Utility script for "
                                     "calculating statistics on labeled data.")
    parser.add_argument('--geomean', action='store_const', const=True,
                        default=False, help='Also outputs the geometric mean '
                                            'of all the other means.')
    parser.add_argument('file', nargs='?', type=str)
    args = parser.parse_args()

    infile = None
    if args.file is None:
        infile = sys.stdin
    else:
        infile = open(args.file, 'r')

    categories = parse_measurements(infile)
    if args.geomean:
        compute_striped_geomeans(categories)
    widest_key = find_widest_key(categories)
    print_means_and_cis(categories, widest_key)


if __name__ == "__main__":
    main()
