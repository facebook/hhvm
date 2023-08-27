#!/usr/bin/env python3

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
    """Cast an HPHP::Op to an integer."""
    if V('HPHP::Op_count') > 256:
        return op.cast(T('uint16_t'))
    else:
        # This is necessary to avoid unwanted sign extension.
        return op.cast(T('uint8_t'))

@memoized
def op_table(name):
    """Get the symbol `name' as an int8_t[]."""
    try:
        return V(name).address.cast(T('int8_t').pointer())
    except:
        # for some reason some of these tables have no type
        # information.  for those cases, just take the address and
        # cast to a pointer.  Note that this *doesn't* work for the
        # tables with types, because gdb objects to the '&'!
        return gdb.parse_and_eval("(unsigned char*)&'%s'" % (name))

#------------------------------------------------------------------------------
# Bytecode immediates.


@memoized
def iva_imm_types():
    return [V('HPHP::' + t) for t in ['IVA', 'LA', 'ILA', 'IA']]


@memoized
def vec_imm_types():
    # keep this in sync with vec_elm_sizes()
    return [V('HPHP::' + t) for t in ['BLA', 'VSA', 'SLA']]


@memoized
def vec_elm_sizes():
    return [T(t).sizeof for t in [
        'HPHP::Offset',      # BLA
        'HPHP::Id',          # VSA
        'HPHP::StrVecItem',  # SLA
    ]]


@memoized
def tv_iva_mcodes():
    return [V('HPHP::' + t) for t in ['MEC', 'MPC']]


@memoized
def tv_loc_mcodes():
    return [V('HPHP::' + t) for t in ['MEL', 'MPL']]


@memoized
def str_imm_mcodes():
    return [V('HPHP::' + t) for t in ['MET', 'MPT', 'MQT']]


@memoized
def rata_arrs():
    return [V('HPHP::RepoAuthType::Tag::' + o + s + t)
            for o in ['', 'Opt']
            for s in ['', 'S']
            for t in ['Arr', 'VArr', 'DArr', 'Vec', 'Dict', 'Keyset']]


@memoized
def rata_objs():
    return [V('HPHP::RepoAuthType::Tag::' + o + s + t)
            for o in ['', 'Opt']
            for s in ['Exact', 'Sub']
            for t in ['Obj', 'Cls']]


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

    ##
    # HPHP::Op decoding and info.
    #
    @staticmethod
    def decode_op(pc):
        """Decode the HPHP::Op at `pc', returning it as well as the size of its
        encoding."""

        pc = pc.cast(T('uint8_t').pointer())
        raw_val = (pc.dereference()).cast(T('size_t'))

        if raw_val == 0xff and V('HPHP::Op_count') > 256:
            pc += 1
            byte = pc.dereference()
            raw_val += byte
            size = 2
        else:
            size = 1

        return [raw_val.cast(T('HPHP::Op')), size]

    @staticmethod
    def decode_iva(pc):
        """Decode the IVA immediate at `pc', returning a dict with 'value' and
        'size' keys."""

        info = {}

        small = pc.cast(T('unsigned char').pointer()).dereference()

        if small & 0x80:
            large = pc.cast(T('uint32_t').pointer()).dereference()
            info['value'] = ((large & 0xffffff00) >> 1) | (large & 0x7f)
            info['size'] = 4
        else:
            info['value'] = small
            info['size'] = 1

        return info

    @staticmethod
    def decode_named_local(pc):
        """Decode the NLA immediate at `pc', returning a dict with 'value' and
        'size' keys."""

        info = {}
        name = HHBC.decode_iva(pc)
        size = name['size']
        locId = HHBC.decode_iva(pc + size)
        size += locId['size']
        info['size'] = size
        info['value'] = '%s (name: %s)' % (
            str(locId['value'].cast(T('int32_t'))),
            str(name['value'].cast(T('int32_t')) - 1)
        )
        return info

    @staticmethod
    def decode_ba(pc):
        """Decode the BA immediate at `pc', returning a dict with 'value' and
        'size' keys."""

        info = {}

        v = pc.cast(T('uint32_t').pointer()).dereference()
        info['value'] = v
        info['size'] = 4

        return info

    @staticmethod
    def op_name(op):
        """Return the name of HPHP::Op `op'."""

        table_name = 'HPHP::opcodeToNameTable'
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

        table_name = 'HPHP::numImmediatesTable'
        return op_table(table_name)[as_idx(op)]

    @staticmethod
    def imm_type(op, arg):
        """Return the type of the arg'th immediate for HPHP::Op `op'."""

        op_count = V('HPHP::Op_count')

        table_name = 'HPHP::immTypeTable'
        # This looks like an int8_t[6][op_count], but in fact, it's actually an
        # int8_t[op_count][6], as desired.
        table_type = T('int8_t').array(6 - 1).array(op_count - 1).pointer()
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

        elif immtype == V('HPHP::NLA'):
            info = HHBC.decode_named_local(ptr)

        elif immtype in vec_imm_types():
            elm_size = vec_elm_sizes()[vec_imm_types().index(immtype)]
            vec_size = HHBC.decode_iva(ptr)
            num_elms = vec_size['value']
            if immtype == V('HPHP::BLLA'):
                num_elms = (num_elms + 7) / 8

            info['size'] = vec_size['size'] + elm_size * num_elms
            info['value'] = '<vector>'

        elif immtype == V('HPHP::KA'):
            ptr = ptr.cast(T('unsigned char').pointer())

            mcode = ptr.dereference().cast(T('HPHP::MemberCode'))
            ptr += 1

            imm_info = {}

            if mcode in tv_iva_mcodes():
                imm_info = HHBC.decode_iva(ptr)
                imm_info['kind'] = 'iva'

            if mcode in tv_loc_mcodes():
                imm_info = HHBC.decode_named_local(ptr)
                imm_info['kind'] = 'local'

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
            imm = imm.cast(T('unsigned short'))
            size = 1
            if imm == 0xff:
                imm += (ptr + size).cast(T('unsigned char').pointer()).dereference()
                size += 1

            imm = (imm >> 2) | (imm << 14)
            radb = V('HPHP::kRATArrayDataBit')

            tag = (imm & ~radb).cast(T('HPHP::RepoAuthType::Tag'))
            high_bit = (imm & radb)

            if tag in rata_arrs():
                if high_bit:
                    size += HHBC.decode_iva(ptr + size)['size']
            elif tag in rata_objs():
                size += HHBC.decode_iva(ptr + size)['size']

            info['size'] = size
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
        elif immtype == V('HPHP::ITA'):
            flags = ptr.cast(T('unsigned char').pointer()).dereference()
            size = 1
            itid = HHBC.decode_iva(ptr + size)
            size += itid['size']
            kid = HHBC.decode_iva(ptr + size)
            size += kid['size']
            vid = HHBC.decode_iva(ptr + size)
            size += vid['size']

            fstr = 'BaseConst ' if flags & V('HPHP::IterArgs::BaseConst') else ''
            istr = str(int(itid['value']))
            kstr = ' k:' + str(kid['value'] - 1) if kid['value'] > 0 else ''
            vstr = ' v:' + str(int(vid['value']))
            info['size'] = size
            info['value'] = fstr + istr + kstr + vstr

        elif immtype == V('HPHP::FCA'):
            flags = ptr.cast(T('uint8_t').pointer()).dereference()
            size = 1

            flagList = []
            if flags & 0x1: flagList.append('Unpack')
            if flags & 0x2: flagList.append('Generics')
            if flags & 0x4: flagList.append('LockWhileUnwinding')
            if flags & 0x8: flagList.append('SkipRepack')

            iva = HHBC.decode_iva(ptr + size)
            numArgs = int(iva['value'])
            size += iva['size']

            numRets = 1
            if flags & V('HPHP::FCallArgsFlags::HasInOut'):
                iva = HHBC.decode_iva(ptr + size)
                numRets = int(iva['value'])
                size += iva['size']

            if flags & V('HPHP::FCallArgsFlags::EnforceInOut'):
                size += (numArgs + 7) // 8

            asyncEagerOffset = ''
            if flags & V('HPHP::FCallArgsFlags::HasAsyncEagerOffset'):
                off = HHBC.decode_ba(ptr + size)
                asyncEagerOffset = ' aeo:' + str(off['value'])
                size += off['size']

            context = ''
            if flags & V('HPHP::FCallArgsFlags::ExplicitContext'):
                id = (ptr + size).cast(T('uint32_t').pointer()).dereference()
                context = ' context:' + str(HHBC.try_lookup_litstr(id))
                size += 4

            info['size'] = size
            info['value'] = ('<' + ','.join(flagList) + '> '
                             + str(numArgs) + ' ' + str(numRets)
                             + asyncEagerOffset + context)

        else:
            table_name = 'HPHP::immSizeTable'
            if immtype >= 0:
                size = op_table(table_name)[immtype.cast(T('size_t'))]

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

        if as_idx(op) >= V('HPHP::Op_count'):
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

If two arguments are provided, the first is interpreted as the start
PC, and the second, as the number of opcodes to print, or the end
address if it's > 0xffffffff.  Subsequent calls to `hhx' may omit these
argument to print the same number of opcodes starting wherever the
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
            self.end = None
        else:
            self.bcpos = argv[0]
            self.bcoff = 0
            if argv[1] > 0xffffffff:
                self.end = argv[1].cast(T('void').pointer())
                self.count = int(self.end) - int(self.bcpos)
            else:
                self.end = None
                self.count = int(argv[1])

        bctype = T('HPHP::PC')
        self.bcpos = self.bcpos.cast(bctype)

        bcstart = self.bcpos - self.bcoff

        for _i in xrange(0, self.count):
            if self.end is not None and self.bcpos >= self.end:
                self.bcpos = None
                break

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
