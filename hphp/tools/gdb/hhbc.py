"""
GDB commands for inspecting HHVM bytecode.
"""

from compatibility import *

import gdb
import unit
from gdbutils import *
from lookup import lookup_litstr


#------------------------------------------------------------------------------
# HPHP::Op -> int16_t table helpers.

def as_idx(op):
    """Cast an HPHP::Op to a uint16_t."""
    return op.cast(T('uint16_t'))

@memoized
def op_table(name):
    """Get the symbol `name' as an int8_t[]."""
    return gdb.parse_and_eval("&'" + name + "'").cast(T('int8_t').pointer())


#------------------------------------------------------------------------------
# Bytecode immediates.

@memoized
def iva_imm_types():
    return [V('HPHP::' + t) for t in ['IVA', 'LA', 'IA', 'CAR', 'CAW']]

@memoized
def vec_imm_types():
    # keep this in sync with vec_elm_sizes()
    return [V('HPHP::' + t) for t in ['BLA', 'VSA', 'SLA', 'I32LA', 'BLLA']]

@memoized
def vec_elm_sizes():
    return [T(t).sizeof for t in [
        'HPHP::Offset',      # BLA
        'HPHP::Id',          # VSA
        'HPHP::StrVecItem',  # SLA
        'uint32_t',          # I32LA
        'uint8_t'            # BLLA
    ]]

@memoized
def iter_table_types():
    return [V('HPHP::' + t) for t in ['ILA']]

@memoized
def cell_loc_mcodes():
    return [V('HPHP::' + t) for t in ['MEC', 'MPC', 'MEL', 'MPL']]

@memoized
def str_imm_mcodes():
    return [V('HPHP::' + t) for t in ['MET', 'MPT', 'MQT']]

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

    ##
    # HPHP::Op decoding and info.
    #
    @staticmethod
    def decode_op(pc):
        """Decode the HPHP::Op at `pc', returning it as well as the size of its
        encoding."""

        pc = pc.cast(T('uint8_t').pointer())
        raw_val = (pc.dereference()).cast(T('size_t'))

        if (raw_val == 0xff):
            pc += 1
            byte = pc.dereference()
            raw_val += byte

        return [raw_val.cast(T('HPHP::Op')), 2 if raw_val >= 0xff else 1]

    @staticmethod
    def decode_iva(pc):
        """Decode the IVA immediate at `pc', returning a dict with 'value' and
        'size' keys."""

        info = {}

        small = pc.cast(T('unsigned char').pointer()).dereference()

        if small & 0x80:
            large = pc.cast(T('uint32_t').pointer()).dereference()
            info['value'] = ((large & 0xffffff00) >> 1) | (large & 0x7f);
            info['size'] = 4
        else:
            info['value'] = small
            info['size'] = 1

        return info

    @staticmethod
    def op_name(op):
        """Return the name of HPHP::Op `op'."""

        table_name = 'HPHP::opcodeToName(HPHP::Op)::namesArr'
        table_type = T('char').pointer().pointer()
        return op_table(table_name).cast(table_type)[as_idx(op)]

    @staticmethod
    def try_lookup_litstr(imm):
        """Return the literal string corresponding to the litstr ID `imm'.  If
        we can't resolve it, just return `imm'."""

        if unit.curunit is None:
            return imm

        litstr = lookup_litstr(imm, unit.curunit)
        return string_data_val(rawptr(litstr))

    ##
    # Opcode immediate info.
    #
    @staticmethod
    def num_imms(op):
        """Return the number of immediates for HPHP::Op `op'."""

        table_name = 'HPHP::numImmediates(HPHP::Op)::values'
        return op_table(table_name)[as_idx(op)]

    @staticmethod
    def imm_type(op, arg):
        """Return the type of the arg'th immediate for HPHP::Op `op'."""

        op_count = V('HPHP::Op_count')

        table_name = 'HPHP::immType(HPHP::Op, int)::argTypes'
        # This looks like an int8_t[5][op_count], but in fact, it's actually an
        # int8_t[op_count][5], as desired.
        table_type = T('int8_t').array(5 - 1).array(op_count - 1).pointer()
        table = op_table(table_name).cast(table_type).dereference()

        immtype = table[as_idx(op)][arg]
        return immtype.cast(T('HPHP::ArgType'))

    @staticmethod
    def imm_info(ptr, immtype):
        """Return the size and value of the immediate at `ptr' with immediate
        type `immtype'.
        """
        info = {}

        if immtype in iva_imm_types():
            info = HHBC.decode_iva(ptr)

        elif immtype in vec_imm_types():
            elm_size = vec_elm_sizes()[vec_imm_types().index(immtype)]
            vec_size = HHBC.decode_iva(ptr)
            num_elms = vec_size['value']
            if immtype == V('HPHP::BLLA'):
                num_elms = (num_elms + 7) / 8

            info['size'] = vec_size['size'] + elm_size * num_elms
            info['value'] = '<vector>'

        elif immtype in iter_table_types():
            info['size'] = 0
            info['value'] = '<vector>'

            size = HHBC.decode_iva(ptr)
            info['size'] += size['size']
            ptr += size['size']

            for _x in range(0, size['value']):
                itertype = HHBC.decode_iva(ptr)
                ptr += itertype['size']
                info['size'] += itertype['size']
                iterid = HHBC.decode_iva(ptr)
                ptr += iterid['size']
                info['size'] += iterid['size']
                if itertype['value'] == V('HPHP::KindOfLIter'):
                    localid = HHBC.decode_iva(ptr)
                    ptr += localid['size']
                    info['size'] += localid['size']

        elif immtype == V('HPHP::KA'):
            ptr = ptr.cast(T('unsigned char').pointer())

            mcode = ptr.dereference().cast(T('HPHP::MemberCode'))
            ptr += 1

            imm_info = {}

            if mcode in cell_loc_mcodes():
                imm_info = HHBC.decode_iva(ptr)
                imm_info['kind'] = 'iva'

            elif mcode == V('HPHP::MEI'):
                t = T('int64_t')
                imm_info['size'] = t.sizeof
                imm_info['kind'] = 'int64'
                imm_info['value'] = ptr.cast(t.pointer()).dereference()

            elif mcode in str_imm_mcodes():
                t = T('HPHP::Id')
                imm_info['size'] = t.sizeof
                imm_info['kind'] = 'litstr'
                raw = ptr.cast(t.pointer()).dereference()
                imm_info['value'] = HHBC.try_lookup_litstr(raw)

            elif mcode == V('HPHP::MW'):
                imm_info['size'] = 0
                imm_info['kind'] = 'iva'
                imm_info['value'] = 0

            info['size'] = 1 + imm_info['size']
            info['value'] = '%s:%s=%s' % (
                str(mcode)[len('HPHP::'):],
                imm_info['kind'],
                str(imm_info['value'])
            )

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

        elif immtype == V('HPHP::LAR'):
            first = ptr.cast(T('unsigned char').pointer()).dereference()
            if first & 0x1:
                first_type = T('int32_t')
            else:
                first_type = T('unsigned char')

            count = (ptr.cast(T('unsigned char').pointer())
                     + first_type.sizeof).dereference()
            if count & 0x1:
                count_type = T('int32_t')
            else:
                count_type = T('unsigned char')

            first = ptr.cast(first_type.pointer()).dereference()
            count = (ptr.cast(T('unsigned char').pointer())
                     + first_type.sizeof).cast(count_type.pointer()).dereference()

            info['size'] = first_type.sizeof + count_type.sizeof
            info['value'] = 'L:' + str(first >> 1) + '+' + str(count >> 1)

        else:
            table_name = 'HPHP::immSize(unsigned char const*, int)::argTypeToSizes'
            if immtype >= 0:
                size = op_table(table_name)[as_idx(immtype)]

                uint = T(uints_by_size()[int(size)])
                imm = ptr.cast(uint.pointer()).dereference()
                au = imm.cast(T('HPHP::ArgUnion'))

                info['size'] = size
                info['value'] = au['u_' + str(immtype)[6:]]

                # Try to print out literal strings.
                if immtype == V('HPHP::SA'):
                    info['value'] = HHBC.try_lookup_litstr(info['value'])
            else:
                info['size'] = 0
                info['value'] = None

        return info

    ##
    # Main public interface.
    #
    @staticmethod
    def instr_info(bc):
        op, instrlen = HHBC.decode_op(bc)

        if op <= 0 or op >= V('HPHP::Op_count'):
            print('hhx: Invalid Op %d @ 0x%x' % (op, bc))
            return None

        imms = []

        for i in xrange(0, int(HHBC.num_imms(op))):
            immoff = bc + instrlen
            immtype = HHBC.imm_type(op, i)
            imminfo = HHBC.imm_info(immoff, immtype)

            instrlen += imminfo['size']
            imms.append(imminfo['value'])

        return {'op': op, 'len': instrlen, 'imms': imms}


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

        bctype = T('HPHP::PC')
        self.bcpos = self.bcpos.cast(bctype)

        bcstart = self.bcpos - self.bcoff

        for _i in xrange(0, self.count):
            instr = HHBC.instr_info(self.bcpos)
            if instr is None:
                print('hhx: Bytecode dump failed')
                break

            name = HHBC.op_name(instr['op']).string()

            start_addr = bcstart.cast(T('void').pointer())

            out = "%s+%d: %s" % (str(start_addr), self.bcoff, name)
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
