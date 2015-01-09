# @lint-avoid-pyflakes2

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import argparse
import os.path
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from glob import glob
from itertools import compress
from operator import not_

verbose = False

def run_test_program(files, program):
    """
    Generate all the .out files.
    """
    def run(f):
        with open(f + '.out', 'w') as outfile:
            test_dir, test_name = os.path.split(f)
            hh_flags = get_hh_flags(test_dir)
            cmd = [program, test_name] + hh_flags
            if verbose:
                print('Executing', ' '.join(cmd))
            subprocess.call(cmd, stdout=outfile, stderr=outfile, cwd=test_dir)

    executor = ThreadPoolExecutor(max_workers=48)
    futures = [executor.submit(run, f) for f in files]

    # this is equivalent to calling wait(), but will also raise exceptions if
    # the future raises
    for f in futures:
        f.result()

def check_results(fnames):
    """
    Check that the .out files match the .exp files.
    """
    results = [check_result(fname) for fname in fnames]
    success = all(results)
    total = len(fnames)
    if success:
        print("All %d tests passed!" % total)
    else:
        failures = list(compress(fnames, map(not_, results)))
        print("Failures:\n" + " ".join(failures))
        for f in failures:
            print("mv {0}.out {0}.exp".format(f))
        print()
        for f in failures:
            print_file(f + '.out')
        print("Failed %d out of %d tests." % (len(failures), total))
    return success

def check_result(fname):
    try:
        with open(fname + '.exp') as fexp:
            exp = fexp.read()
    except FileNotFoundError:
        exp = ''
    with open(fname + '.out') as fout:
        out = fout.read()
    return exp == out

def print_file(fname):
    print('%s:' % fname)
    with open(fname) as f:
        print(f.read())

def get_hh_flags(test_dir):
    path = os.path.join(test_dir, 'HH_FLAGS')
    if not os.path.isfile(path):
        if verbose:
            print("No HH_FLAGS file found")
        return []
    with open(path) as f:
        return f.read().strip().split(' ')

def list_test_files(root):
    if os.path.isfile(root):
        return [root] if root.endswith('.php') else []
    elif os.path.isdir(root):
        result = []
        for child in os.listdir(root):
            if child != 'disabled':
                result.extend(list_test_files(os.path.join(root, child)))
        return result
    else:
        raise Exception('Could not find test file or directory at %s' %
            args.test_path)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument(
            'test_path',
            help='A file or a directory. ')
    parser.add_argument('--program', type=os.path.abspath)
    parser.add_argument('--verbose', action='store_true')
    parser.epilog = "%s looks for a file named HH_FLAGS in the same directory" \
                    " as the test files it is executing. If found, the " \
                    "contents will be passed as arguments to " \
                    "<program>." % parser.prog
    args = parser.parse_args()

    verbose = args.verbose

    if not os.path.isfile(args.program):
        raise Exception('Could not find program at %s' % args.program)

    files = list_test_files(args.test_path)

    # The .exp files that describe expected error output hardcode the path of
    # the source files, so we chdir to ensure that they match.
    run_test_program(files, args.program)
    if not check_results(files):
        sys.exit(1)
