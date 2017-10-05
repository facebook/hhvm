#!/usr/bin/env python3

import re
import os
import os.path as path
import sys
import subprocess
import time
import libfb.py.pathutils as pathutils
import tempfile
import json


def split_as_files(lines, size, utf8=True):
    """take a source of lines and a batch size,
    write the contents to a series of files in a temporary directory
    and then return the name of each file as it is completed"""
    if lines in (list, tuple):
        lines = iter(lines)
    assert hasattr(lines, '__iter__') and not isinstance(lines, (str, bytes))
    assert size >= 1

    tempdir_path = tempfile.mkdtemp()
    name = None
    nonzero_items = False
    nth_file = 1
    try:
        while True:
            name = path.join(tempdir_path, ("batch-%08d.txt" % nth_file))
            nth_file += 1
            nonzero_items = False
            print(name, nth_file)
            with open(name, "wb") as fh:
                for _x in range(size):
                    line = next(lines)
                    nonzero_items = True
                    l = line.strip()
                    if utf8 and not isinstance(l, bytes):
                        l = bytes(l, 'utf-8')
                    fh.write(l)
                    fh.write(b"\n")
            yield name
    except StopIteration:
        pass
    if nonzero_items:
        yield name


devnull_source = open(os.devnull, 'rb')
devnull_sink = open(os.devnull, 'wb')


def get_ocaml_build_rule_output_path(build_target, folder="bin"):
    # some_path/foo.par -> some_path/foo/foo.opt
    out = re.sub(
        r'/([^/]*)[.]par$',
        r'/\1/\1.opt',
        pathutils.get_build_rule_output_path(build_target).replace(
            '/gen/',
            '/' + folder + "/",
        )
    )
    return bytes(out, 'UTF-8')


def walkdir_skip_hidden(directory, extension=None):
    """get non-hidden files recursively under `directory',
    not following symlinks. Files (hidden or otherwise) where
    any parent directory is hidden are also excluded.

    An `extension', if provided, restricts the files yielded to those
    with the extension.
    """
    for root, dirs, files in os.walk(directory):
        # remove hidden directories
        for subdir in dirs:
            if subdir[0] == ".":
                dirs.remove(subdir)

        for name in files:
            (_, ext) = path.splitext(name)
            fullpath = path.join(root, name)
            ruled_out = extension and ext != extension
            if not ruled_out:
                yield fullpath


def devserver_clean_environment(fbsource_path=None, www_path=None):
    "clean the environment on a dev server in preparation for running this test"
    old_cwd = os.getcwd()
    try:
        if fbsource_path is None:
            fbsource_path = path.join(os.getenv("HOME"), "fbsource")
        if www_path is None:
            www_path = path.join(os.getenv("HOME"), "www")

        os.chdir(fbsource_path)
        subprocess.check_call(["buck", "clean"])
        os.chdir("fbcode")
        subprocess.check_call(["buck", "build", "//hphp/hack/src:hh_single_compile"])

        os.chdir(www_path)
        subprocess.check_call(["hg", "checkout", "master"])
        subprocess.check_call(["arc", "pull"])
    finally:
        os.chdir(old_cwd)
    return None


def run_hh_single_compile(www_path=None):
    """run the hh_single_compile over the www director
    processing one file at a time.
    """
    if www_path is None:
        www_path = path.join(os.getenv("HOME"), "www")

    start = None
    hh_single_compile_path = get_ocaml_build_rule_output_path(
        r'@hphp/hack/src:hh_single_compile',
        folder='gen',
    )

    with tempfile.NamedTemporaryFile() as tempf:
        for filepath in walkdir_skip_hidden(www_path, extension=".php"):
            tempf.write(bytes(filepath + "\n", "UTF-8"))
        tempf.flush()

        start = time.time()
        with open(tempf.name, "rb") as fh:
            it = split_as_files(fh, 1000)
            for filepath in it:
                subprocess.check_call(
                    [hh_single_compile_path, "--input-file-list", filepath],
                    stdin=devnull_source,
                    stdout=devnull_sink,
                    stderr=None,
                )

    stop = time.time()

    result = {
        "parse_www_time": (stop - start),
        "parse_www_batch_size": 1000,
    }
    try:
        subprocess.check_call([
            "scribe_cat",
            "perfpipe_hh_ffp_cold_start",
            json.dumps(result)
        ])
    except OSError as e:
        print("got an os error!")
        print(e)

    print("elapsed time %s" % (stop - start))


def main(argv):
    devserver_clean_environment()
    run_hh_single_compile()


if __name__ == "__main__":
    main(sys.argv)
