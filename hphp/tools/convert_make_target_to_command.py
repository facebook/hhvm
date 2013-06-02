#!/bin/env python
#
# Helper for running hphp/test in fbconfig builds. Examples:
#
# ./convert_make_target_to_command.py SlowVM
# ./convert_make_target_to_command.py QuickRepoJit
# ./convert_make_target_to_command.py ZendJitIR
#
# Components are:
#
#    SuiteName [VM|Jit|JitIR] [<blank>|Repo] [-SubSuiteName]
#

import re
import os
import sys

suites = {
    'Slow': 'slow',
    'Quick': 'quick',
    'Zend': 'zend/good',
    'Facebook': '../facebook/test/',
}
modes = {
    'JitIR': 'hhir',
    'Jit': 'jit',
    'VM': 'interp',
}

home = os.getenv('HPHP_HOME')
root = home + '/' + os.getenv('FBMAKE_BIN_ROOT', '_bin')

def main():
    if len(sys.argv) < 2:
        print "%s [Quick|Slow|Zend][<blank>|Repo][VM|Jit|JitIR][-SubSuiteName]" % sys.argv[0]
        return

    arg = sys.argv[1]
    for suite, dir in suites.items():
        if arg.startswith(suite):
            arg = arg.replace(suite, '')

            repo = ''
            if arg.startswith('Repo'):
                arg = arg.replace('Repo', '')
                repo = '-r'

            for mode, vq in modes.items():
                if arg.startswith(mode):
                    arg = arg.replace(mode, '')

                    subpath = ''
                    if len(arg):
                        if arg[0] == '-':
                            subpath = '/' + camel_to_slash(arg[1:])
                        else:
                            raise Exception('Extra? "%s"' % arg)

                    path = relative_path('../test/' + dir + subpath)
                    return [relative_path('../test/run'), path, '-m', vq, repo]

            raise Exception('Unknown mode "%s"' % arg)

    raise Exception('Unknown Suite "%s"' % arg)

def camel_to_slash(name):
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1/\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1/\2', s1).lower()

def relative_path(path):
    """Given a path relative to this file, returns a path relative to cwd"""
    to_file = os.path.join(os.path.realpath(os.path.dirname(__file__)), path)
    return os.path.relpath(to_file)

print ' '.join(main())
