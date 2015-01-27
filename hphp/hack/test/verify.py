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

def run_test_program(files, program, out_ext, get_flags):
    """
    Generate all the output files.
    """
    def run(f):
        with open(f + out_ext, 'w') as outfile:
            test_dir, test_name = os.path.split(f)
            flags = get_flags(test_dir)
            cmd = [program, test_name] + flags
            if verbose:
                print('Executing', ' '.join(cmd))
            subprocess.call(cmd, stdout=outfile, stderr=outfile, cwd=test_dir)

    executor = ThreadPoolExecutor(max_workers=48)
    futures = [executor.submit(run, f) for f in files]

    # this is equivalent to calling wait(), but will also raise exceptions if
    # the future raises
    for f in futures:
        f.result()

def check_results(fnames, out_ext, expect_ext):
    """
    Check that the output files match the expected output files.
    """
    results = [check_result(fname, out_ext, expect_ext) for fname in fnames]
    success = all(results)
    total = len(fnames)
    if success:
        print("All %d tests passed!" % total)
    else:
        failures = list(compress(fnames, map(not_, results)))
        print("Failures:\n" + " ".join(failures))
        for f in failures:
            print("mv {0}{1} {0}{2}".format(f, out_ext, expect_ext))
        print()
        for f in failures:
            print_file(f + expect_ext)
        print("Failed %d out of %d tests." % (len(failures), total))
    return success

def check_result(fname, out_ext, expect_ext):
    try:
        with open(fname + expect_ext) as fexp:
            exp = fexp.read()
    except FileNotFoundError:
        exp = ''
    with open(fname + out_ext) as fout:
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

def files_with_ext(files, ext):
    """
    Returns the set of filenames in :files that end in :ext
    """
    result = set()
    for f in files:
        prefix, suffix = os.path.splitext(f)
        if suffix == ext:
            result.add(prefix)
    return result

def list_test_files(root, disabled_ext):
    if os.path.isfile(root):
        if root.endswith('.php'):
            return [root]
        else:
            return []
    elif os.path.isdir(root):
        result = []
        children = os.listdir(root)
        disabled = files_with_ext(children, disabled_ext)
        for child in children:
            if child != 'disabled' and child not in disabled:
                result.extend(list_test_files(os.path.join(root, child),
                    disabled_ext))
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
    parser.add_argument('--out-extension', type=str, default='.out')
    parser.add_argument('--expect-extension', type=str, default='.exp')
    parser.add_argument('--disabled-extension', type=str,
            default='.no_typecheck')
    parser.add_argument('--verbose', action='store_true')
    parser.add_argument('--flags', nargs=argparse.REMAINDER)
    parser.epilog = "Unless --flags is passed as an argument, "\
                    "%s looks for a file named HH_FLAGS in the same directory" \
                    " as the test files it is executing. If found, the " \
                    "contents will be passed as arguments to " \
                    "<program>." % parser.prog
    args = parser.parse_args()

    verbose = args.verbose

    if not os.path.isfile(args.program):
        raise Exception('Could not find program at %s' % args.program)

    files = list_test_files(args.test_path, args.disabled_extension)

    flags_cache = {}

    def get_flags(test_dir):
        if args.flags is not None:
            return args.flags
        else:
            if test_dir not in flags_cache:
                flags_cache[test_dir] = get_hh_flags(test_dir)
            return flags_cache[test_dir]

    run_test_program(files, args.program, args.out_extension, get_flags)
    if not check_results(files, args.out_extension, args.expect_extension):
        sys.exit(1)
