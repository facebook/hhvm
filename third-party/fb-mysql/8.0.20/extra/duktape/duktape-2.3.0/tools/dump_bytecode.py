#!/usr/bin/env python2
#
#  Utility to dump bytecode into a human readable form.
#

import os
import sys
import struct
import optparse

def decode_string(buf, off):
    strlen, = struct.unpack('>L', buf[off:off+4])
    off += 4
    strdata = buf[off:off+strlen]
    off += strlen

    return off, strdata

def sanitize_string(val):
    # Don't try to UTF-8 decode, just escape non-printable ASCII.
    def f(c):
        if ord(c) < 0x20 or ord(c) > 0x7e or c in '\'"':
            return '\\x%02x' % ord(c)
        else:
            return c
    return "'" + ''.join(map(f, val)) + "'"

def decode_sanitize_string(buf, off):
    off, val = decode_string(buf, off)
    return off, sanitize_string(val)

def dump_function(buf, off, ind):
    count_inst, count_const, count_funcs = struct.unpack('>LLL', buf[off:off+12])
    off += 12
    print('%sInstructions: %d' % (ind, count_inst))
    print('%sConstants: %d' % (ind, count_const))
    print('%sInner functions: %d' % (ind, count_funcs))

    # Line numbers present, assuming debugger support; otherwise 0.
    nregs, nargs, start_line, end_line = struct.unpack('>HHLL', buf[off:off+12])
    off += 12
    print('%sNregs: %d' % (ind, nregs))
    print('%sNargs: %d' % (ind, nargs))
    print('%sStart line number: %d' % (ind, start_line))
    print('%sEnd line number: %d' % (ind, end_line))

    compfunc_flags, = struct.unpack('>L', buf[off:off+4])
    off += 4
    print('%sduk_hcompiledfunction flags: 0x%08x' % (ind, compfunc_flags))

    for i in xrange(count_inst):
        ins, = struct.unpack('>L', buf[off:off+4])
        off += 4
        print('%s  %06d: %08lx' % (ind, i, ins))

    print('%sConstants:' % ind)
    for i in xrange(count_const):
        const_type, = struct.unpack('B', buf[off:off+1])
        off += 1

        if const_type == 0x00:
            off, strdata = decode_sanitize_string(buf, off)
            print('%s  %06d: %s' % (ind, i, strdata))
        elif const_type == 0x01:
            num, = struct.unpack('>d', buf[off:off+8])
            off += 8
            print('%s  %06d: %f' % (ind, i, num))
        else:
            raise Exception('invalid constant type: %d' % const_type)

    for i in xrange(count_funcs):
        print('%sInner function %d:' % (ind, i))
        off = dump_function(buf, off, ind + '  ')

    val, = struct.unpack('>L', buf[off:off+4])
    off += 4
    print('%s.length: %d' % (ind, val))
    off, val = decode_sanitize_string(buf, off)
    print('%s.name: %s' % (ind, val))
    off, val = decode_sanitize_string(buf, off)
    print('%s.fileName: %s' % (ind, val))
    off, val = decode_string(buf, off)  # actually a buffer
    print('%s._Pc2line: %s' % (ind, val.encode('hex')))

    while True:
        off, name = decode_string(buf, off)
        if name == '':
            break
        name = sanitize_string(name)
        val, = struct.unpack('>L', buf[off:off+4])
        off += 4
        print('%s_Varmap[%s] = %d' % (ind, name, val))

    num_formals, = struct.unpack('>L', buf[off:off+4])
    off += 4
    if num_formals != 0xffffffff:
        print('%s_Formals: %d formal arguments' % (ind, num_formals))
        for idx in xrange(num_formals):
            off, name = decode_string(buf, off)
            name = sanitize_string(name)
            print('%s_Formals[%d] = %s' % (ind, idx, name))
    else:
        print('%s_Formals: absent' % ind)

    return off

def dump_bytecode(buf, off, ind):
    sig, = struct.unpack('B', buf[off:off+1])
    print('%sSignature byte: 0x%02x' % (ind, sig))
    off += 1
    if sig == 0xff:
        raise Exception('pre-Duktape 2.2 0xFF signature byte (signature byte is 0xBF since Duktape 2.2)')
    if sig != 0xbf:
        raise Exception('invalid signature byte: %d' % sig)

    off = dump_function(buf, off, ind + '  ')

    return off

def main():
    parser = optparse.OptionParser()
    parser.add_option('--hex-decode', dest='hex_decode', default=False, action='store_true', help='Input file is ASCII hex encoded, decode before dump')
    (opts, args) = parser.parse_args()

    with open(args[0], 'rb') as f:
        d = f.read()
        if opts.hex_decode:
            d = d.strip()
            d = d.decode('hex')
    dump_bytecode(d, 0, '')

if __name__ == '__main__':
    main()
