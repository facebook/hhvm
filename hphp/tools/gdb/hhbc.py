"""
GDB commands for inspecting HHVM bytecode.
"""
# @lint-avoid-python-3-compatibility-imports

from os import sys, path

# GDB and Python modules don't play well together.
_localdir = path.dirname(path.realpath(path.expanduser(__file__)))
if sys.path[0] != _localdir:
    sys.path.insert(0, _localdir)

import gdb
from gdbutils import *


#------------------------------------------------------------------------------
# HPHP::Op -> int8_t table helpers.

def as_idx(op):
    """Cast an HPHP::Op to a uint8_t."""
    return op.cast(T('uint8_t'))

@memoized
def op_table(name):
    """Get the symbol `name' as an int8_t[]."""
    return gdb.parse_and_eval("&'" + name + "'").cast(T('int8_t').pointer())


#------------------------------------------------------------------------------
# Bytecode immediates.

@memoized
def iva_imm_types():
    return [V('HPHP::' + t) for t in ['IVA', 'LA', 'IA']]

@memoized
def vec_imm_types():
    return [V('HPHP::' + t) for t in ['MA', 'BLA', 'SLA', 'ILA', 'VSA']]

@memoized
def vec_elm_sizes():
    return [T(t).sizeof for t in [
        'uint8_t',
        'HPHP::Offset',
        'uint64_t',
        'HPHP::Id',
        'HPHP::StrVecItem'
    ]]

@memoized
def rata_arrs():
    return [V('HPHP::RepoAuthType::Tag' + t) for t in
            ['SArr', 'Arr', 'OptSArr', 'OptArr']]

@memoized
def rata_objs():
    return [V('HPHP::RepoAuthType::Tag' + t) for t in
            ['ExactObj', 'SubObj', 'OptExactObj', 'OptSubObj']]

@memoized
def uints_by_size():
    return {1: 'uint8_t',
            2: 'uint16_t',
            4: 'uint32_t',
            8: 'uint64_t'}

class HHBC:
    """
    Namespace for HHBC inspection helpers.
    """

    @staticmethod
    def num_imms(op):
        """Return the number of immediates for HPHP::Op `op'."""

        table_name = 'HPHP::numImmediates(HPHP::Op)::values'
        return op_table(table_name)[as_idx(op)]

    @staticmethod
    def imm_type(op, arg):
        """Return the type of the arg'th immediate for HPHP::Op `op'."""

        table_fmt = 'HPHP::immType(HPHP::Op, int)::arg%dTypes'
        immtype = op_table(table_fmt % (arg))[as_idx(op)]
        return immtype.cast(T('HPHP::ArgType'))

    @staticmethod
    def imm_info(ptr, immtype):
        """Return the size and value of the immediate at `ptr' with immediate
        type `immtype'.
        """
        info = {}

        if immtype in iva_imm_types():
            imm = ptr.cast(T('unsigned char').pointer()).dereference()
            if imm & 0x1:
                imm = ptr.cast(T('int32_t').pointer()).dereference()
                info['size'] = T('int32_t').sizeof
            else:
                info['size'] = T('unsigned char').sizeof

            info['value'] = imm >> 1

        elif immtype in vec_imm_types():
            prefixes = 2 if immtype == V('HPHP::MA') else 1
            elm_size = vec_elm_sizes()[vec_imm_types().index(immtype)]

            num_elms = ptr.cast(T('int32_t').pointer()).dereference()

            info['size'] = prefixes * T('int32_t').sizeof + \
                           elm_size * num_elms
            info['value'] = 'vector'

        elif immtype == V('HPHP::RATA'):
            imm = ptr.cast(T('unsigned char').pointer()).dereference()

            radb = K('HPHP::kRATArrayDataBig')

            tag = (imm & ~radb).cast(T('HPHP::RepoAuthType::Tag'))
            high_bit = (imm & radb)

            if tag in rata_arrs():
                info['size'] = 5 if high_bit else 1
            elif tag in rata_objs():
                info['size'] = 5
            else:
                info['size'] = 1

            info['value'] = 'RAT'

        else:
            table_name = 'HPHP::immSize(HPHP::Op const*, int)::argTypeToSizes'
            if immtype >= 0:
                size = op_table(table_name)[as_idx(immtype)]

                uint = T(uints_by_size()[int(size)])
                imm = ptr.cast(uint.pointer()).dereference()
                au = imm.cast(T('HPHP::ArgUnion'))

                info['size'] = size
                info['value'] = au['u_' + str(immtype)[6:]]
            else:
                info['size'] = 0
                info['value'] = None

        return info

    @staticmethod
    def instr_info(bc, off=0):
        bc = (bc + off).cast(T('HPHP::Op').pointer())
        op = bc.dereference()

        if op <= V('HPHP::OpLowInvalid') or op >= V('HPHP::OpHighInvalid'):
            print "Invalid Op %d @ %p" % (op, bc)
            return 1

        instrlen = 1
        imms = []

        for i in xrange(0, HHBC.num_imms(op)):
            immoff = bc + instrlen
            immtype = HHBC.imm_type(op, i)
            imminfo = HHBC.imm_info(immoff, immtype)

            instrlen += imminfo['size']
            imms.append(imminfo['value'])

        return {'len': instrlen, 'imms': imms}


#------------------------------------------------------------------------------
# hhx command.

class HHXCommand(gdb.Command):
    """Print $arg1 bytecodes starting from $arg0.

If only one argument is provided, if it's in the range for bytecode
allocations (i.e., > 0xffffffff), print the bytecode at $arg0.  Otherwise,
print $arg0 bytecodes starting from wherever the previous call to `hhx'
left off.
    """

    def __init__(self):
        super(HHXCommand, self).__init__('hhx', gdb.COMMAND_DATA)
        self.bcpos = None

    def invoke(self, args, from_tty):
        argv = [gdb.parse_and_eval(arg) for arg in gdb.string_to_argv(args)]

        if len(argv) == 0:
            if not self.bcpos:
                print 'hhx: No bytecode specified.'
                return
        elif len(argv) == 1:
            if argv[0] > 0xffffffff:
                self.bcpos = argv[0]
                self.bcoff = 0
                self.count = 1
            else:
                self.count = int(argv[0])
        else:
            self.bcpos = argv[0]
            self.bcoff = 0
            self.count = int(argv[1])

        bctype = gdb.lookup_type('HPHP::Op').const().pointer()
        self.bcpos = self.bcpos.cast(bctype)

        op_names = gdb.parse_and_eval(
            "(char **)*(uint32_t*)('HPHP::opcodeToName(HPHP::Op)' + 10)")

        for i in xrange(0, self.count):
            instr = HHBC.instr_info(self.bcpos)

            idx = as_idx(self.bcpos.dereference())
            out = "[%d] %s" % (self.bcoff, op_names[idx].string())
            for imm in instr['imms']:
                out += ' <' + str(imm) + '>'
            print out

            self.bcpos += instr['len']
            self.bcoff += instr['len']

HHXCommand()
