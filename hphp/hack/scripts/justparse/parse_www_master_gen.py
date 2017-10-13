#!/usr/bin/env python3
import sys
import json
import shlex
import io


# This script prints a combined config to stdout. Determinator steps in
# LegoCastle (and possibly other places) require an array of job descriptions
# to be written to stdout in one of the steps This script accomplishes this
# using `echo_this' to produce a short shell snippet that evaluates to a given
# string, which is itself a dumped json config.
#
# To test this script, run `./parse_www_master_gen.py > job.py && scutil create
# job.py' or equivalent


assert sys.version_info[0] >= 3, \
    "Python 2 does not have shlex.quote, also this is a Python3 script."
shell_quote = shlex.quote


def echo_this(s):
    """produce a shell fragment that when fed to /bin/sh -c, produces s"""
    # use printf instead of echo because echo does not behave consistently
    return 'printf "%%s" %s' % shell_quote(s)


def python_this(s):
    """produce a shell fragment that runs the str, shell-escaped,
    with python3 -c"""
    return 'python3 -c %s' % shell_quote(s)


def bash_this(s):
    """produce a shell fragment that runs the string str inside a fresh bash.
    This works around potential strange options that are set in the topmost
    bash like POSIX-compatibility mode, -e or similar."""
    return 'bash -c %s' % shell_quote(s)


build_files = None
with io.open("buck_build_consumer.sh", mode="r", encoding="utf-8") as fh:
    build_files = fh.read()


print_timing_as_json = None
with io.open("consumer_print_timing.py", mode="r", encoding="utf-8") as fh:
    print_timing_as_json = fh.read()


produce_list_of_files = None
with io.open("produce_list_of_files.sh", mode="r", encoding="utf-8") as fh:
    produce_list_of_files = fh.read()


concatenate_files_php = \
    r"""(
        echo "<?hh";
        cat ./files.txt.TEMP | \
            xargs -I% -n1 sh -c "cat \"\$1\"; echo; echo" sh % | \
            perl -ne "print unless /^[<][?]/ or /^namespace/ or /^use/" \
        ) > ./long.php.TEMP"""


extract_timing_info = \
    r"""(
        time ./hh_single_compile.opt.SANDCASTLE --debug-time ./long.php.TEMP
    ) 1>/dev/null 2>./timing.txt.TEMP"""


builder = [
    {
        "alias": "parse-www-job-builder",
        "capabilities": {
            "type": "lego",
            "vcs": "fbcode-fbsource"
        },
        "command": "SandcastleUniversalCommand",
        "hash": "master",
        "scheduleType": "continuous",
        "user": "gregorynisbet",
        "oncall": "hack",
        "args": {
            "name": "sample_name",
            "steps": [
                {
                    "name": "compile",
                    "shell": bash_this(build_files)
                },
                {
                    "name": "artifact step",
                    "shell": ":",
                    "artifacts": [
                        {
                            "artifact_db": True,
                            "name": "hh_single_compile.opt",
                            "paths": ["hh_single_compile.opt.SANDCASTLE"]
                        },
                        {
                            "artifact_db": True,
                            "name": "parse_www_job.par",
                            "paths": [
                                "parse_www_job.par.SANDCASTLE"
                            ]
                        }
                    ]
                }
            ]
        }
    }
]


consumer = [
    {
        "alias": "parse-www-job-consumer",
        "capabilities": {
            "type": "lego",
            "vcs": "hg"
        },
        "command": "SandcastleUniversalCommand",
        "hash": "master",
        "scheduleType": "continuous",
        "user": "gregorynisbet",
        "oncall": "hack",
        "args": {
            "artifacts": [
                {
                    "required": True,
                    "artifact_db": True,
                    "name": "hh_single_compile.opt",
                    "base_paths": [
                        "hh_single_compile.opt.SANDCASTLE"
                    ]
                },
                {
                    "required": True,
                    "artifact_db": True,
                    "name": "parse_www_job.par",
                    "base_paths": [
                        "parse_www_job.par.SANDCASTLE"
                    ]
                }
            ],
            "name": "timing",
            "steps": [
                {
                    "name": "generate random list of files",
                    "shell": bash_this(produce_list_of_files)
                },
                {
                    "name": "generate long php file, removing <?hh etc",
                    "shell": bash_this(concatenate_files_php)
                },
                {
                    "name": "get timing info",
                    "shell": bash_this(extract_timing_info)
                },
                {
                    "name": "show timing info",
                    "shell": python_this(print_timing_as_json)
                }
            ]
        }
    }
]


builder_consumer = builder + consumer


config = [
    {
        "alias": "parse-www-job-master",
        "capabilities": {
            "type": "lego",
            "vcs": "fbcode-fbsource"
        },
        "command": "SandcastleUniversalCommand",
        "hash": "master",
        "scheduleType": "continuous",
        "user": "gregorynisbet",
        "oncall": "hack",
        "args": {
            "name": "determinator",
            "steps": [
                {
                    "name": "determinator: compute jobs",
                    "shell": echo_this(json.dumps(builder_consumer)),
                    "determinator": True
                }
            ]
        }
    }
]


print(json.dumps(config))
