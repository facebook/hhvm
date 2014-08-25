"""
GDB commands related to the HHVM stack.
"""
# @lint-avoid-python-3-compatibility-imports

import os
import gdb
from gdbutils import *


#------------------------------------------------------------------------------
# `walkstk' command.

class WalkstkCommand(gdb.Command):
    """Traverse the interleaved VM and native stacks.

The output backtrace has the following format:

    #<bt frame> <sp> @ <rip>: <function> [at <filename>:<line>]

Filename and line number information is only included for C++ functions.
"""

    def __init__(self):
        super(WalkstkCommand, self).__init__('walkstk', gdb.COMMAND_STACK)


    def print_frame(self, i, sp, rip, func, loc):
        out = '#%-2d %s @ 0x%x: %s' % (i, str(sp), int(str(rip)), func)

        # Munge and print the code location if we have one.
        if loc is not None and loc.symtab is not None:
            filename = loc.symtab.filename

            if 'hphp' in filename:
                head, base = os.path.split(filename)
                _, basedir = os.path.split(head)
                filename = 'hphp/.../' + basedir + '/' + base

            out += ' at ' + filename + ':' + str(loc.line)

        print(out)


    def invoke(self, args, from_tty):
        argv = parse_argv(args)

        if len(argv) > 2:
            print('Usage: walkstk [sp] [rip]')

        # Set sp = $rbp.
        sp_type = T('uintptr_t').pointer()
        sp = gdb.parse_and_eval('$rbp').cast(sp_type)
        if len(argv) >= 1:
            sp = argv[0].cast(ptr_type)

        # Set rip = $rip.
        rip_type = T('uintptr_t')
        rip = gdb.parse_and_eval('$rip').cast(rip_type)
        if len(argv) == 2:
            rip = argv[1].cast(rip_type)

        try:
            mcg = V('HPHP::jit::mcg')
            tc_base = mcg['code']['m_base']
            tc_end = tc_base + mcg['code']['m_codeSize']
        except:
            mcg = None

        i = 0
        native_frame = gdb.newest_frame()
        skip_tc = False  # Only used when we can't find HPHP::mcg.

        # Munge `sp' so that it looks like the stack pointer that would point
        # to it if we had another frame---this lets us promote the "increment"
        # to the beginning of the loop, so that we don't miss the final frame.
        sp = (sp, rip)

        while sp:
            rip = sp[1]
            sp = sp[0].cast(sp_type)

            func = '<unknown>'
            loc = None

            if mcg is not None:
                in_tc = rip >= tc_base and rip < tc_end
            elif not skip_tc:
                # TC frames look like unnamed normal native frames.
                next_frame = native_frame.older()
                in_tc = (next_frame is not None and
                         next_frame.name() is None and
                         next_frame.type() == gdb.NORMAL_FRAME)
            else:
                in_tc = False
                skip_tc = False

            # Try to get the PHP function name from the ActRec at %sp if we're
            # executing in the TC.
            if in_tc:
                ar_type = T('HPHP::ActRec').pointer()
                sd_type = T('HPHP::StringData').pointer()
                try:
                    name = sp.cast(ar_type)['m_func']['m_fullName']
                    name = name['m_raw'].cast(sd_type)['m_data'].string()
                    if len(name):
                        func = '[PHP] ' + name + '()'
                    else:
                        func = '[PHP] <psuedomain>'
                except:
                    if mcg is None:
                        skip_tc = True

            # Pop native frames until we find our current %sp.
            else:
                inlines = 0

                while (native_frame is not None
                       and rip != native_frame.pc()):
                    if inlines > 0 and native_frame.name() is not None:
                        self.print_frame(i, '{inline frame}', rip,
                                         native_frame.name(),
                                         native_frame.find_sal())
                    i += 1
                    inlines += 1
                    native_frame = native_frame.older()

                func = native_frame.name() + '()'
                loc = native_frame.find_sal()

            self.print_frame(i + 1 if in_tc else i, sp, rip, func, loc)

WalkstkCommand()
