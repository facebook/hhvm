#!/usr/bin/env python
"""Various utilities to compute confidence intervals.

"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import math

# Table from http://en.wikipedia.org/wiki/Student's_t-distribution
# We assume a 95% confidence interval.
T_TABLE = [
    (1, 12.71),
    (2, 4.303),
    (3, 3.182),
    (4, 2.776),
    (5, 2.571),
    (6, 2.447),
    (7, 2.365),
    (8, 2.306),
    (9, 2.262),
    (10, 2.228),
    (11, 2.201),
    (12, 2.179),
    (13, 2.160),
    (14, 2.145),
    (15, 2.131),
    (16, 2.120),
    (17, 2.110),
    (18, 2.101),
    (19, 2.093),
    (20, 2.086),
    (21, 2.080),
    (22, 2.074),
    (23, 2.069),
    (24, 2.064),
    (25, 2.060),
    (26, 2.056),
    (27, 2.052),
    (28, 2.048),
    (29, 2.045),
    (30, 2.042),
    (40, 2.021),
    (50, 2.009),
    (60, 2.000),
    (80, 1.990),
    (100, 1.984),
    (120, 1.980),
    (float('+Inf'), 1.960),
]

def t_score(degrees_of_freedom):
    """Returns the 95% bi-directional t-score for the first entry in the table
    with more than the requested degrees of freedom.

    """
    for entry in T_TABLE:
        entry_df, score = entry
        if entry_df >= degrees_of_freedom:
            return score
    raise RuntimeError("Should never be reached")

def arith_mean(samples):
    """Computes the arithmetic mean of a set of samples.

    """
    accum = 0.0
    num_samples = float(len(samples))
    for sample in samples:
        accum += float(sample) / num_samples
    return accum

def sample_std_dev(samples):
    """Computes the standard deviation of a set of samples.

    """
    avg = arith_mean(samples)
    err = 0.0
    for sample in samples:
        err += (sample - avg) * (sample - avg) / (len(samples) - 1)
    return math.sqrt(err)

def mean_standard_error(samples):
    """Computes the mean standard error of a set of samples.

    """
    return sample_std_dev(samples) / math.sqrt(len(samples))

def critical_value(samples):
    """Computes the critical value for a set of samples to be used in the
    confidence interval calculation. Assumes a 95% bi-directional confidence
    interval.

    """
    degrees_of_freedom = len(samples) - 1
    return t_score(degrees_of_freedom)

def margin_of_error(samples):
    """Computes the margin of error for a set of samples.

    """
    return critical_value(samples) * mean_standard_error(samples)

def mean_confidence_interval(samples):
    """Returns a tuple of (arithmetic mean, confidence interval) for a set of
    samples.

    """
    return (arith_mean(samples), margin_of_error(samples))
