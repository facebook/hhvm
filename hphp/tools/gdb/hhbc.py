"""
GDB commands for inspecting HHVM bytecode.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

from compatibility import *

import gdb
import unit
from gdbutils import *
from lookup import lookup_litstr


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
    return [V('HPHP::' + t) for t in ['MA', 'BLA', 'ILA', 'VSA', 'SLA']]

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
    return [V('HPHP::RepoAuthType::Tag::' + t) for t in
            ['SArr', 'Arr', 'OptSArr', 'OptArr']]

@memoized
def rata_objs():
    return [V('HPHP::RepoAuthType::Tag::' + t) for t in
            ['ExactObj', 'SubObj', 'OptExactObj', 'OptSubObj']]

@memoized
def uints_by_size():
    return {1: 'uint8_t',
            2: 'uint16_t',
            4: 'uint32_t',
            8: 'uint64_t'}

class HHBC(object):
    """
    Namespace for HHBC inspection helpers.
    """

    @staticmethod
    def op_name(op):
        """Return the name of HPHP::Op `op'."""

        table_name = 'HPHP::opcodeToName(HPHP::Op)::namesArr'
        table_type = T('char').pointer().pointer()
        return op_table(table_name).cast(table_type)[as_idx(op)]

    @staticmethod
    def num_imms(op):
        """Return the number of immediates for HPHP::Op `op'."""

        table_name = 'HPHP::numImmediates(HPHP::Op)::values'
        return op_table(table_name)[as_idx(op)]

    @staticmethod
    def imm_type(op, arg):
        """Return the type of the arg'th immediate for HPHP::Op `op'."""

        table_fmt = 'HPHP::immType(HPHP::Op, int)::arg%dTypes'
        immtype = op_table(table_fmt % arg)[as_idx(op)]
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
                iva_type = T('int32_t')
            else:
                iva_type = T('unsigned char')

            imm = ptr.cast(iva_type.pointer()).dereference()
            info['size'] = iva_type.sizeof
            info['value'] = imm >> 1

        elif immtype in vec_imm_types():
            prefixes = 2 if immtype == V('HPHP::MA') else 1
            elm_size = vec_elm_sizes()[vec_imm_types().index(immtype)]

            num_elms = ptr.cast(T('int32_t').pointer()).dereference()

            info['size'] = prefixes * T('int32_t').sizeof + \
                           elm_size * num_elms
            info['value'] = '<vector>'

        elif immtype == V('HPHP::RATA'):
            imm = ptr.cast(T('unsigned char').pointer()).dereference()

            radb = V('HPHP::kRATArrayDataBit')

            tag = (imm & ~radb).cast(T('HPHP::RepoAuthType::Tag'))
            high_bit = (imm & radb)

            if tag in rata_arrs():
                info['size'] = 5 if high_bit else 1
            elif tag in rata_objs():
                info['size'] = 5
            else:
                info['size'] = 1

            info['value'] = str(tag)[len('HPHP::RepoAuthType::Tag::'):]

        else:
            table_name = 'HPHP::immSize(HPHP::Op const*, int)::argTypeToSizes'
            if immtype >= 0:
                size = op_table(table_name)[as_idx(immtype)]

                uint = T(uints_by_size()[int(size)])
                imm = ptr.cast(uint.pointer()).dereference()
                au = imm.cast(T('HPHP::ArgUnion'))

                info['size'] = size
                info['value'] = au['u_' + str(immtype)[6:]]

                # Try to print out literal strings.
                if immtype == V('HPHP::SA') and unit.curunit is not None:
                    litstr = lookup_litstr(info['value'], unit.curunit)
                    info['value'] = litstr['m_data']
            else:
                info['size'] = 0
                info['value'] = None

        return info

    @staticmethod
    def instr_info(bc):
        bc = bc.cast(T('HPHP::Op').pointer())
        op = bc.dereference()

        if op <= V('HPHP::OpLowInvalid') or op >= V('HPHP::OpHighInvalid'):
            print('Invalid Op %d @ %p' % (op, bc))
            return 1

        instrlen = 1
        imms = []

        for i in xrange(0, int(HHBC.num_imms(op))):
            immoff = bc + instrlen
            immtype = HHBC.imm_type(op, i)
            imminfo = HHBC.imm_info(immoff, immtype)

            instrlen += imminfo['size']
            imms.append(imminfo['value'])

        return {'len': instrlen, 'imms': imms}


#------------------------------------------------------------------------------
# `hhx' command.

class HHXCommand(gdb.Command):
    """Print an HHBC stream.

If two arguments are provided, the first is interpreted as the start PC, and
the second, as the number of opcodes to print.  Subsequent calls to `hhx' may
omit these argument to print the same number of opcodes starting wherever the
previous call left off.

If only a single argument is provided, if it is in the range for bytecode
allocations (i.e., > 0xffffffff), it replaces the saved PC and defaults the
count to 1 before printing.  Otherwise, it replaces the count and the PC
remains where it left off after the previous call.
"""

    def __init__(self):
        super(HHXCommand, self).__init__('hhx', gdb.COMMAND_DATA)
        self.bcpos = None

    @errorwrap
    def invoke(self, args, from_tty):
        argv = parse_argv(args)

        if len(argv) == 0:
            if not self.bcpos:
                print('hhx: No bytecode specified.')
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

        bctype = T('HPHP::Op').const().pointer()
        self.bcpos = self.bcpos.cast(bctype)

        bcstart = self.bcpos - self.bcoff

        for i in xrange(0, self.count):
            instr = HHBC.instr_info(self.bcpos)
            name = HHBC.op_name(self.bcpos.dereference()).string()

            out = "%s+%d: %s" % (str(bcstart), self.bcoff, name)
            for imm in instr['imms']:
                if type(imm) is str:
                    pass
                elif imm.type == T('uint8_t'):
                    imm = imm.cast(T('uint32_t'))
                elif imm.type == T('char').pointer():
                    imm = '"' + imm.string() + '"'
                out += ' ' + str(imm)
            print(out)

            self.bcpos += instr['len']
            self.bcoff += instr['len']

HHXCommand()
