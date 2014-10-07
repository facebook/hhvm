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

def run_typechecker(files, hh_single_type_check, hh_flags):
    """
    Generate all the .out files.
    """
    def run(f):
        with open(f + '.out', 'w') as outfile:
            cmd = [hh_single_type_check, f] + hh_flags
            if verbose:
                print('Executing', ' '.join(cmd))
            subprocess.call(cmd, stdout=outfile, stderr=outfile)

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

def get_hh_flags():
    if not os.path.isfile('HH_FLAGS'):
        if verbose:
            print("No HH_FLAGS file found")
        return []
    with open('HH_FLAGS') as f:
        return f.read().strip().split(' ')

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument(
            'test_path',
            help='A file or a directory. '
                 'Note: We do not recurse into subdirectories.')
    parser.add_argument('--hh_single_type_check',
        type=os.path.abspath,
        default=os.path.abspath(os.path.join(
            os.path.dirname(__file__),
            '../../../../../src/hh_single_type_check')))
    parser.add_argument('--verbose', action='store_true')
    parser.epilog = "%s looks for a file named HH_FLAGS in the same directory" \
                    " as the test files it is executing. If found, the " \
                    "contents will be passed as arguments to " \
                    "hh_single_type_check." % parser.prog
    args = parser.parse_args()

    verbose = args.verbose

    if not os.path.isfile(args.hh_single_type_check):
        raise Exception('Could not find hh_single_typecheck at %s' %
            args.hh_single_type_check)

    if os.path.isfile(args.test_path):
        test_dir, test_file = os.path.split(args.test_path)
        files = [test_file]
    elif os.path.isdir(args.test_path):
        test_dir = args.test_path
        files = list(map(os.path.basename, glob('%s/*.php' % test_dir)))
    else:
        raise Exception('Could not find test file or directory at %s' %
            args.test_path)

    # The .exp files that describe expected error output hardcode the path of
    # the source files, so we chdir to ensure that they match.
    os.chdir(test_dir)
    run_typechecker(files, args.hh_single_type_check, get_hh_flags())
    if not check_results(files):
        sys.exit(1)
