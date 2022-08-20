#!/usr/bin/env python
# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


import os

try:
    from setuptools import Extension, setup
except ImportError:
    from distutils.core import Extension, setup

watchman_src_dir = os.environ.get("CMAKE_CURRENT_SOURCE_DIR")
if watchman_src_dir is None:
    watchman_src_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..")

# Setuptools is very picky about the path on Windows. They have to be relative
# paths, and on Windows that means we have to be on the same drive as the source
# files. Otherwise it is impossible to obtain a relative path across different
# drives. However this has an implication that we will not be able to build this
# package outside the repository. Not great but it works.
py_dir = os.path.join(watchman_src_dir, "watchman", "python")
if os.name == "nt":
    os.chdir(py_dir)
    py_dir = os.path.relpath(py_dir)


def srcs(names):
    """transform a list of sources to be relative to py_dir"""
    return ["%s/%s" % (py_dir, n) for n in names]


setup(
    name="pywatchman",
    version="1.4.1",
    package_dir={"": py_dir},
    description="Watchman client for python",
    author="Wez Furlong, Rain",
    author_email="wez@fb.com",
    maintainer="Wez Furlong",
    maintainer_email="wez@fb.com",
    url="https://github.com/facebook/watchman",
    long_description="Connect and query Watchman to discover file changes",
    keywords=("watchman inotify fsevents kevent kqueue portfs filesystem watcher"),
    license="BSD",
    packages=["pywatchman"],
    ext_modules=[Extension("pywatchman.bser", sources=srcs(["pywatchman/bser.c"]))],
    platforms="Platform Independent",
    classifiers=[
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Developers",
        "Topic :: System :: Filesystems",
        "License :: OSI Approved :: BSD License",
        "Operating System :: OS Independent",
        "Programming Language :: Python :: 2",
        "Programming Language :: Python :: 2.6",
        "Programming Language :: Python :: 2.7",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.5",
        "Programming Language :: Python :: 3.6",
    ],
    zip_safe=True,
    scripts=srcs(
        [
            "bin/watchman-make",
            "bin/watchman-wait",
            "bin/watchman-replicate-subscription",
        ]
    ),
    test_suite="tests",
)
