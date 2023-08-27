# Copyright 2022-present Facebook. All Rights Reserved

import enum
import lldb
import traceback
import typing

try:
    # LLDB needs to load this outside of the usual Buck mechanism
    import idx
    import lookup
    import utils
except ModuleNotFoundError:
    import hhvm_lldb.idx as idx
    import hhvm_lldb.lookup as lookup
    import hhvm_lldb.utils as utils

"""
The hhx command essentially allows you to dump the bytecode contained
in a region of memory, like we do with the Eval.DumpBytecode flag.
Useful files for reference, including the encoding/decoding algorithms,
are found in:

    //fbcode/hphp/runtime/vm/fcall-args-flags.h
    //fbcode/hphp/runtime/vm/hhbc.cpp
    //fbcode/hphp/runtime/vm/hhbc.h
    //fbcode/hphp/runtime/vm/hhbc-codec.cpp
    //fbcode/hphp/runtime/vm/hhbc-codec.h
    //fbcode/hphp/runtime/vm/hhbc-shared.h
    //fbcode/hphp/runtime/vm/member-key.h
    //fbcode/hphp/runtime/vm/opcodes.h
"""

class InstrInfo(typing.NamedTuple):
    op: lldb.SBValue  # holding an HPHP::Op
    len: int
    imms: typing.List[dict]


#------------------------------------------------------------------------------
# HPHP::Op -> uint16_t table helpers


class OpTableNames(enum.Enum):
    OpcodeToName = 'HPHP::opcodeToNameTable'
    NumImmediates = 'HPHP::numImmediatesTable'
    ImmType = 'HPHP::immTypeTable'
    ImmSize = 'HPHP::immSizeTable'


def op_table(name: OpTableNames, target: lldb.SBTarget) -> lldb.SBValue:
    """ Get the symbol `name` as an int8_t[] """
    try:
        table = utils.Value(name.value, target)
        assert table.type.IsArrayType()
        return table
    except Exception as e:
        utils.debug_print(f"Unable to find op table {name.value}, will cast address to a pointer. Error: {str(e)}")
        # For some reason some of these tables have no type information.
        # For those cases, just take the address and cast to a pointer.
        return target.EvaluateExpression(f"(unsigned char*)& {name.value}")

#------------------------------------------------------------------------------
# Bytecode immediates


def iva_imm_types(target: lldb.SBTarget) -> typing.List[int]:
    return [utils.Enum('HPHP::ArgType', t, target).unsigned for t in ['IVA', 'LA', 'ILA', 'IA']]


def vec_imm_types(target: lldb.SBTarget) -> typing.List[int]:
    # keep this in sync with vec_elm_sizes()
    return [utils.Enum('HPHP::ArgType', t, target).unsigned for t in ['BLA', 'VSA', 'SLA']]


def vec_elm_sizes(target: lldb.SBTarget) -> typing.List[int]:
    return [utils.Type(t, target).size for t in [
        'HPHP::Offset',      # BLA
        'HPHP::Id',          # VSA
        'HPHP::StrVecItem',  # SLA
    ]]

#------------------------------------------------------------------------------
# Member keys

def tv_iva_mcodes(target: lldb.SBTarget) -> typing.List[int]:
    return [utils.Enum('HPHP::MemberCode', t, target).unsigned for t in ['MEC', 'MPC']]


def tv_loc_mcodes(target: lldb.SBTarget) -> typing.List[int]:
    return [utils.Enum('HPHP::MemberCode', t, target).unsigned for t in ['MEL', 'MPL']]


def str_imm_mcodes(target: lldb.SBTarget) -> typing.List[int]:
    return [utils.Enum('HPHP::MemberCode', t, target).unsigned for t in ['MET', 'MPT', 'MQT']]

#------------------------------------------------------------------------------
# Repo authoritative types

def rata_arrs(target: lldb.SBTarget) -> typing.List[int]:
    return [utils.Enum('HPHP::RepoAuthType::Tag', o + s + t, target).unsigned
            for o in ['', 'Opt']
            for s in ['', 'S']
            for t in ['Vec', 'Dict', 'Keyset']]


def rata_objs(target: lldb.SBTarget) -> typing.List[int]:
    return [utils.Enum('HPHP::RepoAuthType::Tag', o + s + t, target)
            for o in ['', 'Opt']
            for s in ['Exact', 'Sub']
            for t in ['Obj', 'Cls']]


def subop_to_name(
        subop: typing.Union[lldb.SBValue, lldb.SBTypeEnumMember],
    ) -> str:
    if isinstance(subop, lldb.SBTypeEnumMember):
        return subop.name

    # If the value is a member of some enum
    if subop.type.enum_members:
        return subop.value

    return str(subop.unsigned)

    # TODO Try and get the enum that the subop is
    # part of with this limited information.
    # Then we can use the rest of this function.
    enum_type_name = None

    subop_names_and_table_starting_indices = {
        "HPHP::InitPropOp": ("InitPropOp", "Static"),
        "HPHP::IsTypeOp": ("IsTypeOp", "Null"),
        "HPHP::FatalOp": ("FatalOp", "Runtime"),
        "HPHP::SetOpOp": ("SetOpOp", "PlusEqual"),
        "HPHP::IncDecOp": ("IncDecOp", "PreInc"),
        "HPHP::BareThisOp": ("BareThisOp", "Notice"),
        "HPHP::SilenceOp": ("SilenceOp", "Start"),
        "HPHP::CollectionType": ("HeaderKind", "Vector"),  # This is the only one with a different table name
        "HPHP::OODeclExistsOp": ("OODeclExistsOp", "Class"),
        "HPHP::ObjMethodOp": ("ObjMethodOp", "NullThrows"),
        "HPHP::SwitchKind": ("SwitchKind", "Unbounded"),
        "HPHP::QueryMOp": ("QueryMOp", "CGet"),
        "HPHP::SetRangeOp": ("SetRangeOp", "Forward"),
        "HPHP::TypeStructResolveOp": ("TypeStructResolveOp", "Resolve"),
        "HPHP::MOpMode": ("MOpMode", "None"),
        "HPHP::ContCheckOp": ("ContCheckOp", "IgnoreStarted"),
        "HPHP::SpecialClsRef": ("SpecialClsRef", "SelfCls"),
        "HPHP::IsLogAsDynamicCallOp": ("IsLogAsDynamicCallOp", "LogAsDynamicCall"),
        "HPHP::ReadonlyOp": ("ReadonlyOp", "Any"),
    }
    enum_name, member = subop_names_and_table_starting_indices[enum_type_name]
    ix = subop.unsigned - utils.Enum("HPHP::" + enum_name, member, subop.target).unsigned
    table = utils.Value(enum_type_name + "_names", subop.target)
    assert table.type.IsArrayType()
    s = idx.at(table, ix)
    return utils.read_cstring(s, 32, subop.process)


#------------------------------------------------------------------------------
# Various helpers

def uints_by_size() -> typing.Dict[int, str]:
    return {
        1: 'uint8_t',
        2: 'uint16_t',
        4: 'uint32_t',
        8: 'uint64_t',
    }


def imm_to_str(imm) -> str:
    if type(imm) is str:
        pass
    elif utils.rawtype(imm.type).name == 'char *':
        imm = imm.summary
    else:
        imm = str(imm.signed)
    return imm


def show_fca_num_args(numArgs: int, boolVecArgs: lldb.SBValue) -> str:
    if boolVecArgs == 0:
        return ""
    out = ""
    tmp = 0
    for i in range(numArgs):
        if (i % 8) == 0:
            tmp = boolVecArgs.deref
            boolVecArgs = utils.ptr_add(boolVecArgs, 1)
        out += "1" if ((tmp.unsigned >> (i % 8)) & 1) else "0"
    return out


class HHBC:
    """
    Namespace for HHBC inspection helpers.
    """

    #
    # HPHP::Op decoding and info.
    #
    @staticmethod
    def decode_op(pc: lldb.SBValue) -> typing.Tuple[lldb.SBValue, int]:
        """Decode the HPHP::Op at `pc`, returning it as well as the size of its encoding """

        raw_val = utils.unsigned_cast(pc.deref, utils.Type('uint32_t', pc.target))

        if raw_val.unsigned == 0xff and utils.Value('HPHP::Op_count', pc.target).unsigned > 256:
            byte = utils.ptr_add(pc, 1).deref
            raw_val = pc.CreateValueFromExpression("tmp", byte.unsigned + 0xff)
            size = 2
        else:
            size = 1
        
        val = utils.unsigned_cast(raw_val, utils.Type('HPHP::Op', pc.target))

        utils.debug_print(f"decode_op(pc=0x{pc.unsigned:x}): val='{val}', size={size}")

        return (val, size)

    @staticmethod
    def decode_iva(pc: lldb.SBValue):
        """ Decode the IVA immediate at `pc`, returning a dict with `value` and `size` keys """

        info = {}
        small = pc.Cast(utils.Type('unsigned char', pc.target).GetPointerType()).deref

        if small.unsigned & 0x80:  # i.e int8_t(*pc) < 0
            utils.debug_print("decode_iva(): large IVA immediate")
            large = pc.Cast(utils.Type('uint32_t', pc.target).GetPointerType()).deref
            info['value'] = large.CreateValueFromExpression("tmp", ((large.unsigned & 0xffffff00) >> 1) | (large.unsigned & 0x7f))
            info['size'] = 4
        else:
            info['value'] = small
            info['size'] = 1

        utils.debug_print(f"decode_iva(pc=0x{pc.unsigned:x}): info={info}")

        return info

    @staticmethod
    def decode_named_local(pc: lldb.SBValue):
        """ Decode the NLA immediate at `pc`, returning a dict with `value` and `size` keys """

        info = {}
        name = HHBC.decode_iva(pc)
        size = name['size']
        locId = HHBC.decode_iva(utils.ptr_add(pc, size))
        size += locId['size']
        info['size'] = size
        info['value'] = 'L:%s:%s' % (
            utils.unsigned_cast(name['value'], utils.Type('int32_t', pc.target)).signed - 1,
            utils.unsigned_cast(locId['value'], utils.Type('int32_t', pc.target)).signed,
        )

        utils.debug_print(f"decode_named_local(pc=0x{pc.unsigned:x}): info={info}")

        return info

    @staticmethod
    def decode_ba(pc: lldb.SBValue):
        """ Decode the BA immediate at `pc`, returning a dict with `value` and `size` keys """

        info = {}
        v = pc.Cast(utils.Type('uint32_t', pc.target).GetPointerType()).deref
        info['value'] = v
        info['size'] = 4

        utils.debug_print(f"decode_ba(pc=0x{pc.unsigned:x}): info={info}")

        return info

    @staticmethod
    def op_name(op: typing.Union[lldb.SBTypeEnumMember, lldb.SBValue], target: lldb.SBValue) -> str:
        """ Return the name of HPHP::Op `op` """

        if isinstance(op, lldb.SBValue):
            name = op.value
        else:
            name = op.name

        if not name:
            table_type = utils.Type('char', target).GetPointerType().GetPointerType()
            table = op_table(OpTableNames.OpcodeToName, target).Cast(table_type)
            name_entry = idx.at(table, op.unsigned)
            name = utils.read_cstring(name_entry, 32, target.process)
        utils.debug_print(f"op_name(op={op}): name={name}")
        return name

    @staticmethod
    def try_lookup_litstr(imm) -> str:
        """ Return the literal string corresponding to the litstr ID `imm`. If we can't resolve it, just return `imm` """

        utils.debug_print(f"try_lookup_litstr(imm={imm})")

        try:
            litstr = lookup.lookup_litstr(imm)
            return utils.string_data_val(utils.rawptr(litstr))
        except Exception:
            return str(imm.unsigned)

    @staticmethod
    def try_lookup_array(imm) -> lldb.SBValue:
        """ Return the array corresponding to the litstr ID `imm`. If we can't resolve it, just return `imm` """

        utils.debug_print(f"try_lookup_array(imm={imm})")

        try:
            return lookup.lookup_array(imm)
        except Exception:
            return imm

    ##
    # Opcode immediate info
    #
    @staticmethod
    def num_imms(op: lldb.SBTypeEnumMember, target: lldb.SBTarget) -> int:
        """ Return the number of immediates for HPHP::Op `op` """

        table = op_table(OpTableNames.NumImmediates, target)
        num_entry = idx.at(table, op.unsigned)

        utils.debug_print(f"num_imms(op='{op}'): num_entry={num_entry.unsigned}")

        return num_entry.unsigned

    @staticmethod
    def imm_type(op: lldb.SBTypeEnumMember, arg: int, target: lldb.SBTarget) -> lldb.SBValue:
        """ Return the ArgType of the arg'th immediate for HPHP::Op `op` """

        # Has type int8_t[op_count][6]
        table = op_table(OpTableNames.ImmType, target)

        imm_entries = idx.at(table, op.unsigned)
        raw_immtype = idx.at(imm_entries, arg)
        # LLDB puts garbage in the upper bits using the normal .Cast() method!
        immtype = utils.unsigned_cast(raw_immtype, utils.Type('HPHP::ArgType', target))
        utils.debug_print(f"imm_type(op='{op}', arg={arg}): immtype={immtype}")
        return immtype

    @staticmethod
    def imm_info(ptr: lldb.SBValue, immtype: lldb.SBValue):
        """ Return the size and value of the immediate at `ptr` with immediate type `immtype` """

        utils.debug_print(f"imm_info(ptr=0x{ptr.unsigned:x}, immtype={immtype})")
    
        target = ptr.target

        info = {}

        if immtype.unsigned in iva_imm_types(target):
            info = HHBC.decode_iva(ptr)
            la = immtype.unsigned in [utils.Enum('HPHP::ArgType', t, target).unsigned for t in ('LA', 'ILA')]
            if la:
                info['value'] = 'L:' + str(info['value'].unsigned)

        elif immtype.unsigned == utils.Enum('HPHP::ArgType', 'NLA', target).unsigned:
            info = HHBC.decode_named_local(ptr)

        elif immtype.unsigned in (vct := vec_imm_types(target)):
            elm_size = vec_elm_sizes(target)[vct.index(immtype.unsigned)]
            vec_size = HHBC.decode_iva(ptr)
            num_elms = vec_size['value'].unsigned

            ptr = utils.ptr_add(ptr, vec_size['size'])
            offset_type = utils.Type("HPHP::Offset", target).GetPointerType()

            elms = []
            for _ in range(num_elms):
                # TODO print the litstrs too, if the op == SSwitch
                offset = utils.unsigned_cast(ptr, offset_type).deref
                # Note: HHVM's bytecode dump shows the absolute offset
                # from the start of the function. Here, we're just
                # showing offset from the current instruction.
                elms.append(str(offset.signed))
                ptr = utils.ptr_add(ptr, offset.size)
            value = "<" + " ".join(elms) + ">"

            info['size'] = vec_size['size'] + elm_size * num_elms
            info['value'] = value

        elif immtype.unsigned == utils.Enum('HPHP::ArgType', 'AA', target).unsigned:
            array_id = utils.unsigned_cast(ptr, utils.Type('HPHP::Id', target).GetPointerType()).deref
            info['size'] = array_id.size
            info['value'] = HHBC.try_lookup_array(array_id)

        elif immtype.unsigned == utils.Enum('HPHP::ArgType', 'KA', target).unsigned:
            ptr = ptr.Cast(utils.Type('unsigned char', target).GetPointerType())

            # No need to use the memberNames and readOnlyNames tables
            # since mcode.value and readonly_op.value should produce their string representations automatically
            mcode = ptr.Dereference().Cast(utils.Type('HPHP::MemberCode', target))
            ptr = utils.ptr_add(ptr, 1)

            imm_info = {}

            get_readonly_op = True

            if mcode.unsigned in tv_iva_mcodes(target):
                imm_info = HHBC.decode_iva(ptr)
                imm_info['kind'] = 'iva'

            elif mcode.unsigned in tv_loc_mcodes(target):
                imm_info = HHBC.decode_named_local(ptr)
                imm_info['kind'] = 'local'

            elif mcode.unsigned == utils.Enum('HPHP::MemberCode', 'MEI', target).unsigned:
                t = utils.Type('int64_t', target)
                imm_info['size'] = t.size
                imm_info['kind'] = 'int64'
                imm_info['value'] = utils.unsigned_cast(ptr, t.GetPointerType()).deref

            elif mcode.unsigned in str_imm_mcodes(target):
                t = utils.Type('HPHP::Id', target)
                imm_info['size'] = t.size
                imm_info['kind'] = 'litstr'
                raw = utils.unsigned_cast(ptr, t.GetPointerType()).deref
                imm_info['value'] = HHBC.try_lookup_litstr(raw)

            elif mcode.unsigned == utils.Enum('HPHP::MemberCode', 'MW', target).unsigned:
                imm_info['size'] = 0
                imm_info['kind'] = 'iva'
                imm_info['value'] = 0
                get_readonly_op = False

            else:
                raise RuntimeError(f"Unrecognized MemberCode: {mcode.unsigned}")

            if get_readonly_op:
                ptr = utils.ptr_add(ptr, imm_info['size'])
                readonly_op = ptr.Dereference().Cast(utils.Type('HPHP::ReadonlyOp', target))

            info['size'] = 1 + imm_info['size'] + int(get_readonly_op)
            info['value'] = '%s:%s %s' % (
                mcode.value[1:],  # Drop leading 'M'
                imm_to_str(imm_info['value']),
                readonly_op.value,
            )

        elif immtype.unsigned == utils.Enum('HPHP::ArgType', 'RATA', target).unsigned:
            tag_type = utils.Type('HPHP::RepoAuthType::Tag', target)
            tag = utils.unsigned_cast(ptr, tag_type.GetPointerType()).deref
            size = 1
            ptr = utils.ptr_add(ptr, size)

            s = ""
            try:
                if tag.unsigned in rata_arrs(target):
                    vid = HHBC.decode_iva(ptr)
                    size += vid['size']
                    s = ' ' + lookup.lookup_array(vid['value'])
                elif tag.unsigned in rata_objs(target):
                    vid = HHBC.decode_iva(ptr)
                    size += vid['size']
                    s = ' ' + lookup.lookup_litstr(vid['value'])
            except Exception:
                pass

            info['size'] = size
            info['value'] = tag.value + s

        elif immtype.unsigned == utils.Enum('HPHP::ArgType', 'LAR', target).unsigned:
            first = HHBC.decode_iva(ptr)
            ptr = utils.ptr_add(ptr, first['size'])
            rest_count = HHBC.decode_iva(ptr)

            info['size'] = first['size'] + rest_count['size']
            info['value'] = 'L:' + str(first.unsigned) + '+' + str(rest_count.unsigned)

        elif immtype.unsigned == utils.Enum('HPHP::ArgType', 'ITA', target).unsigned:
            flags = ptr.Cast(utils.Type('unsigned char', target).GetPointerType()).Dereference()
            size = 1
            itid = HHBC.decode_iva(utils.ptr_add(ptr, size))
            size += itid['size']
            kid = HHBC.decode_iva(utils.ptr_add(ptr, size))
            size += kid['size']
            vid = HHBC.decode_iva(utils.ptr_add(ptr, size))
            size += vid['size']

            fstr = 'BaseConst ' if flags.unsigned & utils.Enum('HPHP::IterArgs::Flags', 'BaseConst', target).unsigned else ''
            istr = str(itid['value'].signed)
            key = kid['value'].signed - 1  # -1 for kNoKey
            kstr = ' K:' + str(key) if (key != -1) else ' NK'
            vstr = ' V:' + str(vid['value'].signed)
            info['size'] = size
            info['value'] = fstr + istr + kstr + vstr

        elif immtype.unsigned == utils.Enum('HPHP::ArgType', 'FCA', target).unsigned:
            flags_type = utils.Type('HPHP::FCallArgsFlags', target)
            flags = ptr.Cast(flags_type.GetPointerType()).Dereference().unsigned
            size = flags_type.size

            utils.debug_print(f"imm_info(imm_type=HPHP::ArgType::FCA): flags=0x{ptr.unsigned:x}, *flags=0b{flags:b}")

            flagList = []
            if flags & utils.Enum('HPHP::FCallArgsFlags', 'HasUnpack', target).unsigned:
                flagList.append('Unpack')
            if flags & utils.Enum('HPHP::FCallArgsFlags', 'HasGenerics', target).unsigned:
                flagList.append('Generics')
            if flags & utils.Enum('HPHP::FCallArgsFlags', 'LockWhileUnwinding', target).unsigned:
                flagList.append('LockWhileUnwinding')
            if flags & utils.Enum('HPHP::FCallArgsFlags', 'SkipRepack', target).unsigned:
                flagList.append('SkipRepack')
            if flags & utils.Enum('HPHP::FCallArgsFlags', 'SkipCoeffectsCheck', target).unsigned:
                flagList.append('SkipCoeffectsCheck')
            if flags & utils.Enum('HPHP::FCallArgsFlags', 'EnforceMutableReturn', target).unsigned:
                flagList.append('EnforceMutableReturn')
            if flags & utils.Enum('HPHP::FCallArgsFlags', 'EnforceReadonlyThis', target).unsigned:
                flagList.append('EnforceReadonlyThis')

            if (numArgsInFlags := (flags >> utils.Value('HPHP::FCallArgs::kFirstNumArgsBit', target).unsigned)):
                utils.debug_print("numArgs stored in upper flag bits")
                numArgs = numArgsInFlags - 1
            else:
                iva = HHBC.decode_iva(utils.ptr_add(ptr, size))
                numArgs = iva['value'].unsigned
                size += iva['size']

            numRets = 1
            if flags & utils.Enum('HPHP::FCallArgsFlags', 'HasInOut', target).unsigned:
                iva = HHBC.decode_iva(utils.ptr_add(ptr, size))
                numRets = iva['value'].unsigned
                size += iva['size']

            inOutArgs = ""
            if flags & utils.Enum('HPHP::FCallArgsFlags', 'EnforceInOut', target).unsigned:
                inOutArgsPtr = utils.ptr_add(ptr, size)
                inOutArgs = show_fca_num_args(numArgs, inOutArgsPtr)
                size += (numArgs + 7) // 8

            readOnlyArgs = ""
            if flags & utils.Enum('HPHP::FCallArgsFlags', 'EnforceReadonly', target).unsigned:
                readOnlyArgsPtr = utils.ptr_add(ptr, size)
                readOnlyArgs = show_fca_num_args(numArgs, readOnlyArgsPtr)
                size += (numArgs + 7) // 8

            asyncEagerOffset = '-'
            if flags & utils.Enum('HPHP::FCallArgsFlags', 'HasAsyncEagerOffset', target).unsigned:
                off = HHBC.decode_ba(utils.ptr_add(ptr, size))
                asyncEagerOffset = 'aeo:' + str(off['value'])
                size += off['size']

            context = ''
            if flags & utils.Enum('HPHP::FCallArgsFlags', 'ExplicitContext', target).unsigned:
                id = utils.ptr_add(ptr, size).Cast(utils.Type('uint32_t', target).GetPointerType()).Dereference()
                context = 'context:' + str(HHBC.try_lookup_litstr(id))
                size += 4

            info['size'] = size
            info['value'] = ('<' + ','.join(flagList) + '> '
                             + str(numArgs) + ' ' + str(numRets)
                             + ' "' + inOutArgs + '"'
                             + ' "' + readOnlyArgs + '"'
                             + ' ' + asyncEagerOffset
                             + ' "' + context + '"')

        elif immtype.unsigned == utils.Enum('HPHP::ArgType', 'OA', target).unsigned:
            subopcode = ptr.Cast(utils.Type('uint8_t', target).GetPointerType()).Dereference()
            info['size'] = 1
            info['value'] = subop_to_name(subopcode)

        else:
            utils.debug_print(f"imm_info(): immtype ({immtype}) didn't match any specially handled ArgType")

            if immtype.unsigned >= 0:
                table = op_table(OpTableNames.ImmSize, target)
                size = idx.at(table, utils.unsigned_cast(immtype, utils.Type('size_t', target))).unsigned
                au = ptr.Cast(utils.Type('HPHP::ArgUnion', target).GetPointerType()).Dereference()

                info['size'] = size
                info['value'] = utils.get(au, 'u_' + str(immtype.value))

                # Try to print out literal strings.
                if immtype.unsigned == utils.Enum('HPHP::ArgType', 'SA', target).unsigned:
                    info['value'] = HHBC.try_lookup_litstr(info['value'])
            else:
                info['size'] = 0
                info['value'] = None

        return info


    @staticmethod
    def instr_info(bc: lldb.SBValue) -> InstrInfo:
        op, instrlen = HHBC.decode_op(bc)
        # Note right now, instrlen is actually the size of the opcode.
        # Later below it's updated as we move past immediates, so it does
        # actually represent the entire "instruction length", or really,
        # the point we're at in the entire instruction encoding.

        op_count = utils.Value('HPHP::Op_count', bc.target)

        if op.unsigned >= op_count.unsigned:
            raise Exception(f'hhx: Invalid Op {op.unsigned} @ 0x{bc.unsigned:x}')

        imms = []

        num_imms = HHBC.num_imms(op, bc.target)
        utils.debug_print(f"instr_info(bc=0x{bc.unsigned:x}): op={op}, opcode_size={instrlen}, num_imms={num_imms}")

        for i in range(0, num_imms):
            utils.debug_print(f"instr_info(): iteration {i+1}/{num_imms}")
            immoff = utils.ptr_add(bc, instrlen)
            immtype = HHBC.imm_type(op, i, bc.target)
            imminfo = HHBC.imm_info(immoff, immtype)

            instrlen += imminfo['size']
            imm = imminfo['value']
            imms.append(imm)

            utils.debug_print(f"instr_info(): immediate #{i+1} = {imm} ({immtype})")

        utils.debug_print(f"instr_info(): instrlen={instrlen}")

        return InstrInfo(op=op, len=instrlen, imms=imms)


class HhxCommand(utils.Command):
    command = "hhx"
    description = "Print an HHBC stream"
    usage = """\
usage: hhx <pc>         (1) if    <pc> >  0xffffffff
       hhx <count>      (2) if <count> <= 0xffffffff
       hhx <pc> <end>   (3) if   <end> >  0xffffffff
       hhx <pc> <n>     (4) if     <n> <= 0xffffffff

If only a single argument is provided, if it is in the range for bytecode
allocations (i.e., > 0xffffffff), it replaces the saved PC and defaults the
count to 1 before printing (1).  Otherwise, it replaces the count and the PC
remains where it left off after the previous call (2).

If two arguments are provided, the first is interpreted as the start
PC, and the second, as the end address if it's > 0xffffffff (3), or the
number of opcodes to print otherwise (4). Subsequent calls to `hhx` may
omit these argument to print the same number of opcodes starting wherever
the previous call left off.
"""

    @classmethod
    def create_parser(cls):
        # Not using a parser
        return cls.default_parser()

    def get_long_help(self):
        return self.usage

    def __init__(self, debugger, internal_dict):
        self.bcpos: typing.Optional[lldb.SBValue] = None
        self.bcoff: int = 0
        self.count: int = 1
        self.end: typing.Optional[lldb.SBValue] = None
        super().__init__(debugger, internal_dict)

    def __call__(self, debugger, command, exe_ctx, result):
        target = exe_ctx.target
        argv = utils.parse_argv(command, target=target)

        if len(argv) == 0:
            if not self.bcpos:
                result.SetError('hhx: No bytecode specified')
                return
        elif len(argv) == 1:
            if argv[0].unsigned > 0xffffffff:
                self.bcpos = argv[0]
                self.bcoff = 0
                self.count = 1
            else:
                self.count = argv[0].unsigned
            self.end = None
        else:
            self.bcpos = argv[0]
            self.bcoff = 0
            if argv[1].unsigned > 0xffffffff:
                self.end = argv[1].Cast(utils.Type('void', target).GetPointerType())
                self.count = self.end.unsigned - self.bcpos.unsigned
            else:
                self.end = None
                self.count = argv[1].unsigned

        utils.debug_print(f"HhxCommand:__init__(): bcpos=0x{self.bcpos.unsigned:x}, bcoff={self.bcoff}, count={self.count}, end={hex(self.end.unsigned) if self.end else None}")

        bctype = utils.Type("uint8_t", target).GetPointerType()  # TODO(T159273123): This was HPHP::PC, but looking up typedefs often fails
        self.bcpos = self.bcpos.Cast(bctype)

        assert self.bcpos.GetError().Success(), f"Unable to cast bcpos: {self.bcpos.GetError()}"

        bcstart = utils.ptr_add(self.bcpos, -self.bcoff)

        for _i in range(0, self.count):
            utils.debug_print(f"\nHhxCommand::__init__(): iteration {_i+1} of a maximum of {self.count}; bcpos=0x{self.bcpos.unsigned:x}")
            if self.end is not None and self.bcpos.unsigned >= self.end.unsigned:
                self.bcpos = None
                break

            try:
                instr = HHBC.instr_info(self.bcpos)
            except Exception as e:
                if utils._Debug:
                    traceback.print_exc()
                result.SetError(f'hhx: Bytecode dump failed. Error: {str(e)}')
                break

            name = HHBC.op_name(instr.op, target)

            start_addr = bcstart.Cast(utils.Type('void', target).GetPointerType())

            out = f"0x{start_addr.unsigned:x}+{self.bcoff}: {name}"
            for imm in instr.imms:
                out += ' ' + imm_to_str(imm)
            result.write(out + "\n")

            self.bcpos = utils.ptr_add(self.bcpos, instr.len)
            self.bcoff += instr.len


def __lldb_init_module(debugger, _internal_dict, top_module=""):
    """ Register the commands in this file with the LLDB debugger.

    Defining this in this module (in addition to the main hhvm module) allows
    this script to be imported into LLDB separately; LLDB looks for a function with
    this name at module load time.

    Arguments:
        debugger: Current debugger object
        _internal_dict: Dict for current script session. For internal use by LLDB only.

    Returns:
        None
    """
    HhxCommand.register_lldb_command(debugger, __name__, top_module)
