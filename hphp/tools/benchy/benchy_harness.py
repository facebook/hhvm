#!/usr/bin/env python
"""Harness for running benchmarks.

This module handles all aspects of running a series of benchmarks with
pre-built executables and emitting the results into a central location for
further processing.

It's currently very focused on HHVM and php-octane, but it could be generalized
because it uses many standard shell features and it leaves the raw benchmark
results in text files for other tools to process.
"""


from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import argparse
import json
import os
import random
import re
import shutil
import subprocess
import sys

import benchy_config as config

class Benchmark(object):
    """A single benchmark invocation.

    """
    def __init__(self, suite, name, path, owner):
        self.name = name
        self.path = path
        self.suite = suite
        self.owner = owner
        self.children = []

    def matches(self, pattern):
        """Returns true if the given pattern matches this Benchmark.

        """
        if re.search(pattern, self.name) is not None:
            return True
        return False

    def __str__(self):
        return '{0.suite}-{0.name}'.format(self)

    def __repr__(self):
        return self.__str__()

class Suite(object):
    """A named group of related benchmarks.

    """
    def __init__(self, name, benchmarks, statistic):
        self.name = name
        self.benchmarks = []
        self.statistic = statistic
        for raw_bench in benchmarks:
            self.benchmarks.append(Benchmark(suite=name,
                                             name=raw_bench['name'],
                                             path=raw_bench['path'],
                                             owner=raw_bench['owner']))

        for benchmark in self.benchmarks:
            if not benchmark.owner:
                continue
            owner = None
            for other in self.benchmarks:
                if benchmark.owner != other.name:
                    continue
                owner = other
                break
            if owner is None:
                msg = "Couldn't find owner {0.owner} of benchmark {0.name}"
                raise RuntimeError(msg.format(benchmark))
            owner.children.append(benchmark)

    def matches(self, pattern):
        """Returns true if the given pattern matches this Suite.

        """
        if re.search(pattern, self.name) is not None:
            return True
        return False

    def __str__(self):
        return "{0.name}".format(self)

    def __repr__(self):
        return self.__str__()


def _unique_id():
    """Returns a unique integer ID to uniquely identify each VM.

    """
    result = _unique_id.next_id
    _unique_id.next_id += 1
    return result
_unique_id.next_id = 1


class VirtualMachine(object):
    """A single named executable with which to run benchmarks and measure.

    """
    def __init__(self, name, path, env, args):
        self.name = name
        self.path = path
        self.uid = _unique_id()
        self.env = env
        self.args = ' '.join(args)

    def __str__(self):
        return "{0.name}".format(self)

    def __repr__(self):
        return self.__str__()
VirtualMachine.pattern = r'(?P<name>[^:]+)(?P<opts_path>(?::[^:]+)+)'


def parse_opts(opts_list):
    """Parses a series of environment variable assignments or command line
    arguments

    """
    env = []
    args = []
    for item in opts_list:
        if item.startswith('-'):
            args.append(item)
        else:
            env.append(item.split('='))
    return (dict(env), args)


def load_benchmark_suites():
    """Loads the benchmark suites to run from the suites.json file.

    """
    json_obj = None
    with open(config.SUITES_PATH, 'r') as suites_file:
        try:
            json_obj = json.load(suites_file)
        except Exception as exc:
            sys.stderr.write("Failed to load suites from JSON file.\n")
            raise exc

    if json_obj['version'] != config.VERSION:
        sys.stderr.write("Unknown version in suites JSON file.\n")
        sys.exit(-1)

    suites = []
    raw_suites = json_obj['suites']
    for raw_suite in raw_suites:
        suites.append(Suite(name=raw_suite['name'],
                            benchmarks=raw_suite['benchmarks'],
                            statistic=raw_suite['statistic']))
    return suites


def _flatten(list_of_lists):
    """Flattens a list of lists.

    """
    result = []
    for elem in list_of_lists:
        result.extend(elem)
    return result


def filter_suites_and_benchmarks(suites, included_suites, included_benchmarks):
    """Filters in specified benchmarks and suites.

    Returns a list of benchmarks whose suite and benchmark names match at
    least one of the specified regexes.

    """
    def matches(thing, patterns):
        """Returns true if the thing matches any of the provided patterns.

        """
        for pattern in patterns:
            if thing.matches(pattern):
                return True
        return False

    filtered_suites = [s for s in suites if matches(s, included_suites)]
    benchmarks = _flatten([suite.benchmarks for suite in filtered_suites])
    return [b for b in benchmarks if matches(b, included_benchmarks)]


def setup_workdir():
    """Deletes any old stale working directory and creates a fresh one.

    """
    if os.path.isfile(config.WORK_DIR):
        msg = "Work directory {0} already exists and is a file"
        raise RuntimeError(msg.format(config.WORK_DIR))
    if os.path.isdir(config.WORK_DIR):
        shutil.rmtree(config.WORK_DIR)
    os.makedirs(config.WORK_DIR)


def warmup_lines_to_chop(benchmark, warmup):
    """Returns the number of lines to be excluded from final measurements.

    """
    # The main benchmark emits its own line.
    lines_to_chop = warmup
    # Each of the child benchmarks emit their own line.
    lines_to_chop += warmup * len(benchmark.children)
    # hhvm_wrapper emits an extra line to let us know its compiling bytecode.
    lines_to_chop += 1
    # Tail 1-indexes line numbers and starts emitting from the nth line.
    lines_to_chop += 1
    return lines_to_chop


def set_env(env):
    """Returns a series of lines to set all the environment variables to their
    corresponding values in env.

    """
    lines = []
    for key, value in env.iteritems():
        lines.append("export {0}={1}".format(key, value))
    return '\n'.join(lines)


def unset_env(env):
    """Returns a series of lines to unset all the environment variables in env.

    """
    lines = []
    for key, _ in env.iteritems():
        lines.append("unset {0}".format(key))
    return '\n'.join(lines)


def single_run(**kwargs):
    """Generates the necessary shell-fu for a single benchmark invocation.

    """
    template = """
    printf "\\033[2K\\r"
    printf "[{idx}/{total}] {vm.name}: {bench.name}"
    printf "<?\\n" > {include}
    printf "include 'util.php';\\n" >> {include}
    printf "include '{bench.path}';\\n" >> {include}
    printf "QueueRuns({extra_iters}, \\${bench.name});\\n" >> {include}
    {setenv}
    {wrapper} --compile --build-root={vm.path} {perf} {vm.args} \\
      -- {harness} > {tmp}
    {unsetenv}
    cat {tmp} | tail -n +{lines_to_chop} >> {runlog}
    """
    lines = template.format(**kwargs).split('\n')[1:-1]
    return [s.strip() for s in lines]


def generate_runscript(vms, benchmarks_to_run, run_perf, warmup, inner, outer):
    """Generates the script that will run all of the benchmarks.

    """
    final_runlist = []
    for virtual_machine in vms:
        for benchmark in benchmarks_to_run:
            # Benchmarks that are run as part of other benchmarks are excluded.
            if benchmark.owner is not None:
                continue
            for _ in range(outer):
                final_runlist.append((virtual_machine, benchmark))
    random.shuffle(final_runlist)

    lines = ['set -e']
    for i in range(len(final_runlist)):
        virtual_machine, benchmark = final_runlist[i]
        runlog_path = config.RUNLOG_PATH + ('.{0.uid}'.format(virtual_machine))
        perf_str = '--perf={base}.{bench}.{vm.uid}'.format(
            base=config.PERF_PATH,
            bench=str(benchmark),
            vm=virtual_machine)
        lines.extend(single_run(
            idx=i + 1,
            total=len(final_runlist),
            vm=virtual_machine,
            bench=benchmark,
            lines_to_chop=warmup_lines_to_chop(benchmark, warmup),
            extra_iters=warmup + inner - 1,
            perf=perf_str if run_perf else '',
            runlog=runlog_path,
            include=config.INCLUDE_PATH,
            wrapper=config.WRAPPER_PATH,
            harness=config.BENCH_ENTRY_PATH,
            tmp=config.TMP_PATH,
            setenv=set_env(virtual_machine.env),
            unsetenv=unset_env(virtual_machine.env)))
    lines.append("printf '\\a\\n'")

    with open(config.RUNSCRIPT_PATH, 'w') as runscript:
        for line in lines:
            runscript.write(line)
            runscript.write('\n')


def execute_runscript():
    """Executes the generated runscript.

    """
    subprocess.call(['sh', config.RUNSCRIPT_PATH])


def parse_virtual_machines(raw_vms):
    """Parses the name and path for each specified VM.

    Provides a default name if none is provided. Doesn't verify that the
    provided path exists.

    """
    vms = []
    for raw_vm in raw_vms:
        result = re.match(VirtualMachine.pattern, raw_vm)
        if result is None:
            raise RuntimeError("Invalid format for VM: %s" % raw_vm)
        name = result.group('name')
        opts_path = result.group('opts_path').split(':')[1:]
        env, args = parse_opts(opts_path[:-1])
        path = str(opts_path[-1])

        vms.append(VirtualMachine(name, path, env, args))
    return vms


def main():
    """Parses arguments and passes them on to all the subsequent phases.

    """
    parser = argparse.ArgumentParser(description="Run some benchmarks.")
    parser.add_argument('--suite', action='append', type=str,
                        help='Run any suite that matches the provided regex')
    parser.add_argument('--benchmark', action='append', type=str,
                        help='Run any benchmark that matches the provided '
                             'regex')
    parser.add_argument('--inner', action='store', type=int, default=4,
                        help='Number of iterations of the benchmark to run '
                             'for each VM instance')
    parser.add_argument('--outer', action='store', type=int, default=4,
                        help='Number of instances of the VM to run for each '
                             'benchmark')
    parser.add_argument('--dry-run', action='store_const', const=True,
                        default=False, help='Don\'t run any benchmarks. Only '
                                            'generate the runscript')
    parser.add_argument('--perf', action='store_const', const=True,
                        default=False, help='Run perf for each benchmark.')
    parser.add_argument('--warmup', action='store', type=int, default=1,
                        help='Number of inner iterations to warmup the VM.')
    parser.add_argument('vm', nargs='+', type=str, metavar='VM',
                        help='VM to benchmark. Consists of NAME:PATH. Can also '
                             'add a colon-separated list of environment '
                             'variables to set when benchmarking this VM. '
                             'E.g. NAME:VAR1=VAL1:VAR2=VAL2:PATH')
    args = parser.parse_args()

    setup_workdir()

    included_suites = args.suite
    if included_suites is None:
        included_suites = [r'.*']

    included_benchmarks = args.benchmark
    if included_benchmarks is None:
        included_benchmarks = [r'.*']

    warmup = args.warmup
    inner = args.inner
    outer = args.outer
    dry_run = args.dry_run
    run_perf = args.perf
    vms = parse_virtual_machines(args.vm)

    suites = load_benchmark_suites()
    benchmarks_to_run = filter_suites_and_benchmarks(suites,
                                                     included_suites,
                                                     included_benchmarks)

    if len(benchmarks_to_run) == 0:
        sys.stderr.write("No benchmarks to run, exiting...\n")
        return

    generate_runscript(vms, benchmarks_to_run, run_perf, warmup, inner, outer)
    if dry_run:
        return
    execute_runscript()


if __name__ == "__main__":
    main()
