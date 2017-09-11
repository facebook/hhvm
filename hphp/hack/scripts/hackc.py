#!/usr/bin/env python

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import argparse
import itertools
import multiprocessing
import os
import signal
import subprocess
import sys
import threading
import time


hh_single_compile = "hh_single_compile.opt"
pool_size = 60
delete_generated_code = True
total_timeout = multiprocessing.Value('i', 0)
timeout_limit = "30s"
list_timeout = multiprocessing.Queue()
file_with_nyi = multiprocessing.Queue()
sort_of_nyis = multiprocessing.Queue()
collect_nyis = False
quiet_mode = False
nyis = {
    "NYI: lval expression": 0,
    "NYI: Folding": 0,
    "NYI: Expected a literal expression in literal_from_expr": 0,
    "NYI: collection_literal in constant folder": 0,
    "NYI: foreach codegen does not support arbitrary lvalues yet": 0,
    "NYI - Xhp attribute hint": 0,
    "NYI: default value closure body": 0,
}



def p(text, bypass_quiet_mode=False):
    if not quiet_mode or bypass_quiet_mode:
        sys.stdout.write(text)
        sys.stdout.flush()


def run_hack(input_filename):
    hack_cl = " ".join(
        ["timeout", timeout_limit, hh_single_compile, input_filename])
    proc = subprocess.Popen(
        hack_cl,
        shell=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)
    (stdoutdata, _stderrdata) = proc.communicate()
    if proc.returncode == 124:
        total_timeout.value += 1
        list_timeout.put(input_filename)
    if collect_nyis:
        file_nyis = [l for l in stdoutdata.split('\n') if "HackCNYI" in l]
        cpt = file_nyis.count("HackCNYI")
        for line in file_nyis:
            found = False
            for ti in nyis:
                if ti in line:
                    cpt -= 1
                    sort_of_nyis.put(ti)
                    found = True
            if not found and cpt > 0:
                file_with_nyi.put(input_filename)
                break


def manage_cli_options():
    global collect_nyis
    global hh_single_compile
    global quiet_mode
    parser = argparse.ArgumentParser(
        description='Hack compiler')
    parser.add_argument(
        'input_f_d', metavar='<directory_path | file_path>', nargs='+',
        help='either files or directories to walk recursively')
    parser.add_argument(
        '-p', '--path',
        nargs=1,
        help='precise which hackc instance to use')
    parser.add_argument(
        '-n', '--collect_nyis',
        action='store_true',
        help='Collect and list files which contains NYIs')
    parser.add_argument(
        '-q', '--quiet',
        action='store_true',
        help='quiet mode')
    args = parser.parse_args()
    collect_nyis = args.collect_nyis
    path = args.path
    quiet_mode = args.quiet
    if path:
        hh_single_compile = path[0]
    return args.input_f_d


def pretty_time_delta(sec):
    from datetime import datetime, timedelta

    sec = timedelta(seconds=sec)
    d = datetime(1, 1, 1) + sec

    def getS(x):
        if x > 0:
            return 's'
        return ''

    def getString(x, word):
        result = ""
        if x > 0:
            result = "{} {}{} ".format(x, word, getS(x))
        return result
    result = "{}{}{}{}".format(
        getString(d.day - 1, "day"),
        getString(d.hour, "hour"),
        getString(d.minute, "minute"),
        getString(d.second, "second"),
    )
    return result


def list_php_files(directory):
    file_paths = []  # List which will store all of the full filepaths.
    collecting_str = "collecting PHP and Hack files: {0}\r"
    cpt = 0
    # Walk the tree.
    for root, _directories, files in os.walk(directory):
        for filename in [file for file in files if file.endswith(".php")]:
            cpt += 1
            # Join the two strings in order to form the full filepath.
            filepath = os.path.abspath(os.path.join(root, filename))
            file_paths.append(filepath)  # Add it to the list.
            p(collecting_str.format(cpt))
    p("\n")
    return file_paths  # Self-explanatory.


def init_worker():
    signal.signal(signal.SIGINT, signal.SIG_IGN)


if __name__ == '__main__':
    files_dir = manage_cli_options()
    files = []
    for v in files_dir:
        if os.path.isfile(v):
            files += [v]
        else:
            files += list_php_files(v)
    pool = multiprocessing.Pool(pool_size, init_worker)
    start_time = time.time()
    pool.map_async(run_hack, files)
    pool.close()
    animate = True

    def animate():
        for c in itertools.cycle(['|', '/', '-', '\\']):
            if animate:
                p('\rcompiling ' + c)
                time.sleep(0.1)
            else:
                return
    t = threading.Thread(target=animate)
    t.start()
    pool.join()
    pool.terminate()
    animate = False
    end_time = time.time()
    p("\rCompiled {} files in {}\033[K\n".format(
        len(files),
        pretty_time_delta(end_time - start_time),
    ))
    p("\t{} timeout after {}\n".format(total_timeout.value, timeout_limit))
    i = 1
    if not list_timeout.empty():
        p("timeouts:\n")
    while not list_timeout.empty():
        filename = list_timeout.get_nowait()
        p("{}\n".format(filename))
        if i % 10 == 0:
            p("\n")
        i += 1
    if collect_nyis:
        while not sort_of_nyis.empty():
            nyis[sort_of_nyis.get()] += 1
        for k, v in nyis.items():
            if v > 0:
                p("{} {}\n".format(v, k))
        if not file_with_nyi.empty():
            p("--> we found unknown NYIs in the following files:\n")
        while not file_with_nyi.empty():
            p("{}\n".format(file_with_nyi.get()))
    p("\n")
