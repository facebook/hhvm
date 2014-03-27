# Copyright (c) 2014, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the "hack" directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.

# HackForHipHop Vim-OmniComplete v0.2

import vim
import re
from hack import Util, Const

from subprocess import Popen, PIPE

class HackOmniComplete(object):
    """
        Find the first non completable character. Not very smart.
        Works on a reversed line.
    """
    matcher = re.compile("^([\w$]+)(.*)$")

    def __init__(self, findstart, base):
        self.findstart = findstart
        self.base = base

    # This thing will pipe the buffer to hh_client, and return its output
    def getLines(self):
      lines = [str(line) for line in vim.current.buffer]
      (row, col) = vim.current.window.cursor
      cur_line = lines[row-1]
      cur_line = cur_line[:col] + self.base + "AUTO332" + cur_line[col:]
      lines[row-1] = cur_line
      input_buffer = "\n".join(lines)

      # Send the buffer to hh_client and return the lines
      args = [
        Const.CLIENT,
        r'--auto-complete'
      ]
      proc = Popen(args, stdin=PIPE, stdout=PIPE, stderr=PIPE)
      proc.stdin.write(input_buffer);
      out = proc.communicate()[0]

      base = self.base

      try:
        proc.kill();
      except:
        pass
      proc = args = None
      #Util.debugf('getLines.txt', locals())

      return [x for x in out.split("\n") if len(x) > 0]

    # This thing will parse all hh_client lines into dictionaries
    def getDicts(self, lines):
      res = []
      for line in lines:
        space = line.find(' ')
        word = None
        menu = None
        if line[0] == '$':
          # Hacky assumption that this is all a variable name
          name_end = len(line)
          if space != -1:
            name_end = space
          word = line[:name_end]
          if not word.startswith(self.base):
            continue
          menu = line[name_end:]
        elif space < 0:
          # looks like a class name!
          word = line
          menu = '' # TODO: add info here!!! class/interface/trait
          if not word.startswith(self.base):
            continue
        else:
          # function or constant or $this->something
          word = line[:space]
          if not word.startswith(self.base):
            continue
          menu = line[space+1:]
          if menu.find('(function(') == 0:
            # remove the function prefix, ex:
            # "function(int, int): RT" => "(int, int): RT"
            menu = menu[9:-1]
          else:
            # Umm, so, this can be a constant OR a field, as in
            # $this->AUTO332 (variables will start without $)
            # so I'll just use 'v' here.
            kind = 'v'

        res.append({'word': word, 'menu': menu})
      return res

    # This thing will transform a python dictionary to a vim dictionary
    def toVimDictionaryString(self, d):
      buffer = []
      for k, v in d.iteritems():
        buffer.append("'%s': '%s'" % (k, v))
      return '{' + ",".join(buffer) + '}'

    # This thing does the autocomplete.
    # Wires the getLines -> getDicts -> toVimDictionaryString
    def completeMain(self):
      lines = self.getLines()
      dicts = self.getDicts(lines)
      vim_res = '[' + ",".join([self.toVimDictionaryString(x) for x in dicts]) + ']'
      #Util.debugf('completeMain.txt', locals())
      vim.command('return %s' % vim_res)

    def findMain(self):
      (row, col) = vim.current.window.cursor
      # Get the current line, up to "col" index, and then reverse it
      line = str(vim.current.buffer[row-1])[:col][::-1]
      # Match all characters until we reach something that's not a name character
      sre = self.matcher.search(line)
      org_start = len(line)
      decrement = 0
      if sre is not None:
        decrement = len(sre.groups()[0])

      start = org_start - decrement
      #Util.debugf('findMain.txt', locals())
      vim.command('return ' + str(start))

    # Entry point
    def main(self):
      if self.findstart:
        self.findMain()
      else:
        self.completeMain()

