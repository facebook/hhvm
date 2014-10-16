"""Convenience wrapper for benchmarking.

Ties together all of the parts of building branches, running benchmarks, and
comparing results.

Expects to be given a number of branches to compare. The script will then
check out each branch, build the branch into its own directory, run the
benchmark harness using the built branches, process the output to get various
statistics, and print the results for easy comparison.
"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import argparse
import os
import shlex
import subprocess

import benchy_config as config

def _unique_id():
    """Returns the next unique integer ID.

    Note: this mirrors the function of the same name in the harness. We should
    probably do something more intelligent than copy and paste, but it works.

    """
    result = _unique_id.next_id
    _unique_id.next_id += 1
    return result
_unique_id.next_id = 1


def verbose():
    """Returns the current verbosity level.

    """
    return verbose.level
verbose.level = 0


def set_verbose_level(level):
    """Sets the verbosity level for debugging.

    """
    verbose.level = level


class Branch(object):
    """A branch within a repository, i.e. the basic unit of comparison."""
    def __init__(self, name):
        self.name = name
        self.uid = _unique_id()

    def build_dir(self):
        """Returns the build directory for this branch.

        """
        return os.path.join(config.BUILD_ROOT, self.name)

    def root_dir(self):
        """Returns the root directory inside the build directory for this
        branch.

        """
        return os.path.join(self.build_dir(), config.BUILD_INTERNAL_PATH)


def parse_branches(raw_branches):
    """Maps branch names and to Branch objects.

    """
    branches = []
    for raw_branch in raw_branches:
        branches.append(Branch(raw_branch))
    return branches


def run_command(cmd, env=None, stdout=None):
    """Runs a command and checks the return code for errors.

    """
    cmd = shlex.split(cmd.encode('utf8'))
    if verbose() >= 1:
        print(cmd)
    subprocess.check_call(cmd, env=env, stdout=stdout)


def build_branches(branches):
    """Builds each of the branches into their own directories.

    """
    env = os.environ.copy()
    for branch in branches:
        run_command('arc feature %s' % branch.name)
        run_command('fbmake clean')

        build_dir = branch.build_dir()
        if os.path.isfile(build_dir):
            os.remove(build_dir)
        if not os.path.exists(build_dir):
            os.makedirs(build_dir)

        env['FBMAKE_BUILD_ROOT'] = build_dir
        run_command('fbmake --build-root "%s" opt -j70' % build_dir, env)


def run_benchmarks(suites, benchmarks, run_perf, inner, outer, branches):
    """Runs the benchmarks on the branches by invoking the harness script.

    """
    benchy_path = config.HARNESS_PATH

    suite_str = ' '.join(["--suite %s" % s for s in suites])
    benchmark_str = ' '.join(["--benchmark %s" % b for b in benchmarks])
    perf_str = '--perf' if run_perf else ''
    inner_str = '' if inner is None else '--inner {0}'.format(inner)
    outer_str = '' if outer is None else '--outer {0}'.format(outer)
    branch_str = ' '.join(["%s:%s" % (b.name, b.root_dir()) for b in branches])

    command = "{harness} {suites} {benchmarks} {perf} {inner} {outer} {branch}"
    run_command(command.format(harness=benchy_path,
                               suites=suite_str,
                               benchmarks=benchmark_str,
                               perf=perf_str,
                               inner=inner_str,
                               outer=outer_str,
                               branch=branch_str))


def process_results(branches, output_mode):
    """Runs statistics on the results and pretty prints them.

    """
    anymean = config.ANYMEAN_PATH
    significance = config.SIGNIFICANCE_PATH
    result_paths = []
    counter = 0

    for branch in branches:
        counter += 1
        runlog = os.path.join(config.WORK_DIR, "runlog.%d" % counter)
        result_path = os.path.join(config.WORK_DIR, branch.name)
        with open(result_path, 'w') as result_file:
            cmd = "{anymean} --geomean {runlog}"
            run_command(cmd.format(anymean=anymean, runlog=runlog),
                        stdout=result_file)
        result_paths.append(result_path)

    cmd = "{significance} --{output_mode} {results}"
    run_command(cmd.format(significance=significance,
                           output_mode=output_mode,
                           results=' '.join(result_paths)))


def main():
    """Parses args and passes them off to the other phases.

    """
    parser = argparse.ArgumentParser(description='Convenience wrapper for '
                                     'benchmarking multiple branches.')
    parser.add_argument('--no-build', action='store_const', const=True,
                        help='Don\'t clean and build.')
    parser.add_argument('--suite', action='append', type=str,
                        help='Run any suite that matches the provided regex')
    parser.add_argument('--benchmark', action='append', type=str,
                        help='Run any benchmark that matches the provided '
                             'regex')
    parser.add_argument('--inner', action='store', type=int,
                        help='Number of iterations of the benchmark to run '
                             'for each VM instance')
    parser.add_argument('--outer', action='store', type=int,
                        help='Number of instances of the VM to run for each '
                             'benchmark')
    parser.add_argument('branch', nargs='+', type=str, metavar='BRANCH',
                        help='Branch to benchmark')
    parser.add_argument('--remarkup', action='store_const', const=True,
                        default=False, help='Spit out the results as Remarkup')
    parser.add_argument('--perf', action='store_const', const=True,
                        default=False, help='Run perf for each VM invocation.')
    parser.add_argument('-v', '--verbose', type=int, default=0,
                        help='Increase verbosity')
    args = parser.parse_args()

    included_suites = args.suite
    if included_suites is None:
        included_suites = []

    included_benchmarks = args.benchmark
    if included_benchmarks is None:
        included_benchmarks = []

    set_verbose_level(args.verbose)
    inner = args.inner
    outer = args.outer
    do_build = args.no_build is None
    run_perf = args.perf
    output_mode = 'remarkup' if args.remarkup else 'terminal'

    branches = parse_branches(args.branch)

    if do_build:
        build_branches(branches)
    run_benchmarks(included_suites,
                   included_benchmarks,
                   run_perf, inner, outer, branches)
    process_results(branches, output_mode)


if __name__ == "__main__":
    main()
