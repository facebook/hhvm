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


def main(argv):
    start = None
    hh_single_compile_path = get_ocaml_build_rule_output_path(
        r'@hphp/hack/src:hh_single_compile',
        folder='gen',
    )
    www_path = path.join(os.getenv("HOME"), "www")

    with tempfile.NamedTemporaryFile() as tempf:
        for filepath in walkdir_skip_hidden(www_path, extension=".php"):
            tempf.write(bytes(filepath + "\n", "UTF-8"))
        tempf.flush()

        start = time.time()
        subprocess.check_call(
            [hh_single_compile_path, "--input-file-list", tempf.name])

    stop = time.time()

    result = {
        "parse_www_time": (stop - start)
    }
    try:
        subprocess.check_call([
            "scribe_cat",
            "perfpipe_hh_ffp_cold_start",
            json.dump(result)
        ])
    except OSError as e:
        print("got an os error!")
        print(e)
        pass

    print("elapsed time %s" % (stop - start))


if __name__ == "__main__":
    main(sys.argv)
