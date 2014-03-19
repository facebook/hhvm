# Copyright (c) 2014, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the "hack" directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.

# HackForHipHop Vim-ColorMode v0.2

import vim
import re
import json

from hack import Util, Const

from subprocess import Popen, PIPE

class HackColorMode(object):

    def __init__(self, path):
        self.path = path
        self.errorMatchers = []

    def getColorInfo(self):
        # Send the buffer to hh_client and return the lines
        args = [
                Const.CLIENT,
                '--json',
                '--color',
                self.path,
                ]
        proc = Popen(args, stdin=PIPE, stdout=PIPE, stderr=PIPE)
        out = proc.communicate()[0]

        try:
            proc.kill();
        except:
            pass

        try:
            return json.loads(out)
        except Exception:
            vim.command("echoerr '[HackColor] Error parsing HH output.'")
            #vim.command("echoerr 'HH Out: {out}'".format(out=out))
            return []


    def trackTheSillyJsonOutput(self, json):
        line = 1
        column = 0
        for record in json:
            type = record[u'color']
            text = record[u'text']
            start_column = 0
            end_column = 0
            # Handle each line separately
            for character in text:
                column += 1
                if character != u'\n':
                    if character != ' ':
                        if start_column == 0:
                            start_column = column - 1
                        end_column = column + 2
                    continue
                if end_column > 0:
                    self.issueMatchCommand(
                            type,
                            line,
                            start_column,
                            end_column,
                    )
                    end_column = start_column = 0
                line += 1
                column = 0

            if end_column > 0:
                self.issueMatchCommand(
                        type,
                        line,
                        start_column,
                        end_column,
                )

    def issueMatchCommand(self, type, sl, sc, ec):
        if type != u'err':
            return
        # add highlight matcher
        matcher = r'\%{line}l\%>{sc}v.\+\%<{ec}v'.format(
                line=sl,
                sc=sc,
                ec=ec,
                )
        self.errorMatchers.append(matcher);

    def applyErrorMatchers(self):
        if not len(self.errorMatchers):
            return
        allMatchers = r'\|'.join(self.errorMatchers)
        vim.command(r''':let b:HackColorMode = matchadd("HackUnsafe", '{matchers}')'''.format(matchers=allMatchers))

    # Run the matching.
    def main(self):
        json = self.getColorInfo()
        if not json:
            vim.command(':close')
            return
        self.trackTheSillyJsonOutput(json)
        self.applyErrorMatchers()

