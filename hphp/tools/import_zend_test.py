#!/usr/bin/env python

import os
import re
import sys

if len(sys.argv) == 1:
    print "Usage: \n\n  %s /tmp/php-5.4.11/Zend/tests/zend_test.phpt ..." % sys.argv[0]
    sys.exit(0)

test_files = []

def split(pattern, str):
    return re.split(r'\n\s*--'+pattern+'--\s*\n', str, 1)

for filename in sys.argv[1:]:
    print "Importing %s" % filename
    zend = file(filename).read()
    boom = split('FILE', zend)
    if len(boom) != 2:
        print "Malformed test, no --FILE--: ", filename
        continue

    name, therest = boom

    boom = split('EXPECT', therest)
    if len(boom) == 2:
        test, exp = boom
    else:
        boom = split('EXPECTF', therest)
        if len(boom) == 2:
            test, exp = boom
        else:
            print "Malformed test, no --EXPECT-- or --EXPECTF--: ", filename
            continue

    dest_filename = 'zend_'+os.path.basename(filename).replace('.phpt', '.php')
    cur_dir = os.path.dirname(__file__)
    dest_dir = os.path.join(cur_dir, '../test/vm')
    full_dest_filename = os.path.join(dest_dir, dest_filename)

    test_files.append(full_dest_filename)

    if 'in %s on' in exp:
        exp = exp.replace('in %s on', 'in hphp/test/vm/%s on' % dest_filename)
        filter_file = full_dest_filename+'.filter'
        if os.path.exists(filter_file):
            os.unlink(filter_file)
        os.symlink('filepath.filter', filter_file)
    exp = exp.replace('Fatal error:', 'HipHop Fatal error:')
    exp = exp.replace('Warning:', 'HipHop Warning:')
    exp = exp.replace('Notice:', 'HipHop Notice:')
    # you'll have to fix up the line %d yourself

    file(full_dest_filename, 'w').write(test)
    file(full_dest_filename+'.exp', 'w').write(exp)

print "\nYou probably have to run \n\n  make verify_quick\n\nthen "+\
      "inspect the .out files and copy them over:\n"

for file in test_files:
    print "  cp %s %s" % (file+".out", file+".exp")
