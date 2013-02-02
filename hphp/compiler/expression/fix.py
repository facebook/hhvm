#!/usr/bin/python

import re
import sys

regex = re.compile(r'(\w+)::\1(\s*)\((.*)\)(\s*):(\s*)Expression\(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES\)', re.M | re.S)

def fix_file(file):
  global regex
  fp = open(file, 'r')
  contents = fp.read()
  rep = regex.sub(r'\1::\1\2(\3)\4:\5Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(\1))',
                  contents)
  fp.close()
  fp = open(file, 'w')
  fp.write(rep)
  fp.close()

for fname in sys.argv[1:]:
  fix_file(fname)
