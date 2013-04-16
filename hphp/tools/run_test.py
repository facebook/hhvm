#!/bin/env python
#
# Helper for running hphp/test in fbconfig builds. Examples:
#
# ./run_test.py SlowVM
# ./run_test.py QuickRepoJit
# ./run_test.py ZendJitIR
#
# Components are:
#
#    SuiteName [VM|Jit|JitIR] [<blank>|Repo] [-SubSuiteName]
#

import re
import os
import subprocess
import sys

suites = {
    'Slow' : 'slow', 
    'Quick' : 'quick',
    'Zend' : 'zend/good',
}
modes = {
    'JitIR' : 'hhir',
    'Jit' : 'jit',
    'VM' : 'interp',
}

home = os.getenv('HPHP_HOME')
root = os.getenv('FBMAKE_BIN_ROOT', home + '/_bin')

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

                    path = 'test/' + dir + subpath
                    cmd = ['test/run', path, '-m', vq, repo]
                    print ' '.join(cmd)
                    return subprocess.call(cmd)

            raise Exception('Unknown mode "%s"' % arg)

    os.chdir(home + '/hphp')
    cmd = sys.argv
    cmd[0] = root + '/hphp/test/test'
    return subprocess.call(cmd)

def camel_to_slash(name):
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1/\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1/\2', s1).lower()

sys.exit(main())
