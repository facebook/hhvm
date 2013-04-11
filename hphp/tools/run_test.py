#!/bin/env python
#
# Helper for running hphp/test in fbconfig builds.
#
# ./run_test.py TestCodeRunVM
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
    'TestCodeRun' : 'tcr',
    'Quick' : 'vm',
    'Zend' : 'zend/good',
}
modes = {
    'JitIR' : 'hhir',
    'Jit' : 'jit',
    'VM' : 'inperp',
}

home = os.getenv('HPHP_HOME')
root = os.getenv('FBMAKE_BIN_ROOT', home + '/_bin')

def main():
    if len(sys.argv) < 2:
        print "%s [TestCodeRun|Quick|Zend][<blank>|Repo][VM|Jit|JitIR][-SubSuiteName]" % sys.argv[0]
        return

    arg = sys.argv[1]
    for suite, dir in suites.items():
        if arg.startswith(suite):
            arg = arg.replace(suite, '')

            repo = ''
            if arg.startswith('Repo'):
                arg = arg.replace('Repo', '')
                repo = '1'

            for mode, vq in modes.items():
                if arg.startswith(mode):
                    arg = arg.replace(mode, '')

                    subpath = ''
                    if len(arg):
                        if arg[0] == '-':
                            subpath = '/' + camel_to_slash(arg[1:])
                        else:
                            raise Exception('Extra? "%s"' % arg)

                    env = {
                        'REPO' : repo,
                        'VQ' : vq,
                        'TEST_PATH' : 'test/' + dir + subpath
                    }
                    print ' '.join([key + '=' + value for key, value in env.items()]) + 'tools/run_verify.sh'
                    env.update(os.environ)
                    subprocess.call('tools/run_verify.sh', env=env)
                    return

            raise Exception('Unknown mode "%s"' % arg)

    os.chdir(home + '/hphp')
    cmd = sys.argv
    cmd[0] = root + '/hphp/test/test'
    subprocess.call(cmd)

def camel_to_slash(name):
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1/\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1/\2', s1).lower()
main()
