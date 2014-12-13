#!/usr/bin/env python

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import re
import sys
import os
import inspect

self = os.path.abspath(inspect.getfile(inspect.currentframe()))
root = os.path.dirname(os.path.dirname(os.path.dirname(self)))
root_re = re.escape(root)

for test in sys.argv[1:]:
    if not test.endswith('.php'):
        print("%s doesn\'t end in .php. Pass the .php file to this script." %
               test)
        sys.exit(1)

    try:
        data = open(test + '.out').read()
    except IOError:
        print("%s.out doesn't exist, skipping" % test)
        continue

    try:
        # the first match has to be in a try incase there is bad unicode
        re.sub('a', r'a', data)
    except UnicodeDecodeError:
        print("%s has invalid unicode, skipping" % test)
        continue

    # try to do relative paths
    data = re.sub('string\(\d+\) "(#\d+) ' + root_re + '(/hphp)?',
                  r'string(%d) "\1 %s',
                  data)
    data = re.sub('string\(\d+\) "' + root_re + '(/hphp)?',
                  r'string(%d) "%s', data)
    data = re.sub(root_re + '(/hphp)?', '%s', data)

    # The debugger prints the path given on the command line, which is often
    # relative. All such debugger tests live under something/debugger/foo.php.
    data = re.sub('[^ ]*/debugger(/[^ ]*.php)', r'%s\1', data)

    # Generator method names are, well, generated!
    # See ParserBase::getAnonFuncName.
    data = re.sub(' [0-9]+_[0-9]+\(', ' %d_%d(', data)

    # Closure names change
    data = re.sub('Closure\$\$[0-9a-f]+\$', 'Closure%s', data)

    # Closure class name
    data = re.sub('string\(7\) "Closure"', 'string(%d) "Closure%s"', data)

    # Left over Closure class names
    data = re.sub('Closure(?!%s)', 'Closure%s', data)

    if '%' in data:
        name = test + '.expectf'
    else:
        name = test + '.expect'

    open(name, 'w').write(data)
    print('Copied %s.out to %s' % (test, name))
