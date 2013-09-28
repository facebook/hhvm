#!/usr/bin/env python

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import re
import sys

for test in sys.argv[1:]:
    if not test.endswith('.php'):
        print ("%s doesn\'t end in .php. Pass the .php file to this script." % test)
        sys.exit(1)

    try:
        data = file(test + '.out').read()
    except IOError:
        print ("%s.out doesn't exist, skipping" % test)
        continue

    try:
        data = re.sub('/data[^ ]*/hphp', '%s', data)
    except UnicodeDecodeError:
        print ("%s has invalid unicode, skipping" % test)
        continue

    # The debugger prints the path given on the command line, which is often
    # relative. All such debugger tests live under something/debugger/foo.php.
    data = re.sub('[^ ]*/debugger(/[^ ]*.php)', r'%s\1', data)

    # Generator method names are, well, generated!
    # See ParserBase::getAnonFuncName.
    data = re.sub(' [0-9]+_[0-9]+\(', ' %d_%d(', data)

    # Closure names change
    data = re.sub('Closure\$\$[0-9a-f]+\$', 'Closure$$%s$', data)
    file(test + '.expectf', 'w').write(data)

    print ('Copied %s.out to %s.expectf' % (test, test))
