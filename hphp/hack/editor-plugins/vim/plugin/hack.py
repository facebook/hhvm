# Copyright (c) 2014, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the "hack" directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.

# HackForHipHop Vim-Hack-Python-Core v0.1

class Const(object):
    CLIENT = r'hh_client'
    DEBUG = False

class Util(object):
    @staticmethod
    def debugf(name, d):
        if not Const.DEBUG:
            return
        debugf = "\n------------------------\n".join(
                [repr(k) + " => " + repr(v) for k,v in d.iteritems()])
        fp = file("/tmp/vimhackdbg-" + name, "w")
        fp.write(debugf)
        fp.close()
        return
