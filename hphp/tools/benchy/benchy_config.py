#!/usr/bin/env python
"""Configuration loader for benchy benchmark harness.

"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import json
import os

def _load():
    """Initializes and returns a singleton config dictionary.

    """
    config = _load.config
    if config is not None:
        return _load.config

    benchy_dir = os.path.dirname(os.path.realpath(__file__))
    tools_dir = os.path.dirname(benchy_dir)
    base_dir = os.path.dirname(tools_dir)
    fbcode_dir = os.path.dirname(base_dir)
    benchmark_dir = os.path.join(base_dir, 'benchmarks', 'php-octane')
    _load.config = {
        'ANYMEAN_PATH': os.path.join(benchy_dir, 'any_mean.py'),
        'BENCHMARK_DIR': benchmark_dir,
        'BENCH_ENTRY_PATH': os.path.join(benchmark_dir, 'harness-run.php'),
        'HARNESS_PATH': os.path.join(benchy_dir, 'benchy_harness.py'),
        'INCLUDE_PATH': os.path.join(benchmark_dir, 'include.php'),
        'SIGNIFICANCE_PATH': os.path.join(benchy_dir, 'significance.py'),
        'SRCROOT_PATH': fbcode_dir,
        'SUITES_PATH': os.path.join(benchmark_dir, 'suites.json'),
        'VERSION': 1,
        'WRAPPER_PATH': os.path.join(tools_dir, 'hhvm_wrapper.php'),
    }

    home_dir = os.path.expanduser('~')
    config_path = os.path.join(home_dir, '.benchy')
    with open(config_path, 'r') as config_file:
        tmp = json.load(config_file)
        work_dir = _load.config['WORK_DIR'] = tmp['work_dir']
        _load.config['BUILD_ROOT'] = tmp['build_dir']
        _load.config['RUNSCRIPT_PATH'] = os.path.join(work_dir, 'runscript')
        _load.config['RUNLOG_PATH'] = os.path.join(work_dir, 'runlog')
        _load.config['PERF_PATH'] = os.path.join(work_dir, 'perf')
        _load.config['TMP_PATH'] = os.path.join(work_dir, 'tmp')
        _load.config['PLATFORM'] = "%s_platform" % tmp['platform']
    return _load.config
_load.config = None


def _get(key):
    """Looks up the given key in the config singleton.

    """
    config = _load()
    if key in config:
        return config[key]
    return None


ANYMEAN_PATH = _get('ANYMEAN_PATH')
BENCHMARK_DIR = _get('BENCHMARK_DIR')
BENCH_ENTRY_PATH = _get('BENCH_ENTRY_PATH')
BUILD_ROOT = _get('BUILD_ROOT')
HARNESS_PATH = _get('HARNESS_PATH')
INCLUDE_PATH = _get('INCLUDE_PATH')
PERF_PATH = _get('PERF_PATH')
PLATFORM = _get('PLATFORM')
RUNLOG_PATH = _get('RUNLOG_PATH')
RUNSCRIPT_PATH = _get('RUNSCRIPT_PATH')
SIGNIFICANCE_PATH = _get('SIGNIFICANCE_PATH')
SRCROOT_PATH = _get('SRCROOT_PATH')
SUITES_PATH = _get('SUITES_PATH')
TMP_PATH = _get('TMP_PATH')
VERSION = _get('VERSION')
WORK_DIR = _get('WORK_DIR')
WRAPPER_PATH = _get('WRAPPER_PATH')


def verbose():
    """Returns the current verbosity level.

    """
    return verbose.level
verbose.level = 0


def set_verbose_level(level):
    """Sets the verbosity level for debugging.

    """
    verbose.level = level
