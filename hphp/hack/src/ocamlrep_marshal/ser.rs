#![allow(non_camel_case_types, non_snake_case, non_upper_case_globals)]
#![allow(clippy::needless_late_init)]

// Initially generatd by c2rust of 'extern.c' at revision:
// `f14c8ff3f8a164685bc24184fba84904391e378e`.

use std::io::Write;

use libc::c_char;
use libc::c_double;
use libc::c_int;
use libc::c_long;
use libc::c_uchar;
use libc::c_uint;
use libc::c_ulong;
use libc::c_void;
use libc::memcpy;
use libc::memset;

use crate::intext::*;

trait WrappingOffset<T> {
    fn wrapping_offset_from(self, origin: *const T) -> isize;
}

impl<T> WrappingOffset<T> for *const T {
    #[inline]
    fn wrapping_offset_from(self, origin: *const T) -> isize {
        let pointee_size = std::mem::size_of::<T>();
        assert!(0 < pointee_size && pointee_size <= isize::max_value() as usize);

        let d = isize::wrapping_sub(self as _, origin as _);
        d.wrapping_div(pointee_size as _)
    }
}

extern "C" {
    fn caml_alloc_string(len: mlsize_t) -> value;
    fn caml_convert_flag_list(_: value, _: *const c_int) -> c_int;
    fn caml_stat_alloc_noexc(_: asize_t) -> caml_stat_block;
    fn caml_failwith(msg: *const c_char) -> !;
    fn caml_invalid_argument(msg: *const c_char) -> !;
    fn caml_raise_out_of_memory() -> !;
    fn caml_stat_calloc_noexc(_: asize_t, _: asize_t) -> caml_stat_block;
    fn caml_stat_free(_: caml_stat_block);
    fn caml_string_length(_: value) -> mlsize_t;
    fn caml_gc_message(_: c_int, _: *const c_char, _: ...);
}

type size_t = c_ulong;
type int64_t = c_long;
type intnat = c_long;
type uintnat = c_ulong;

#[inline]
unsafe fn caml_umul_overflow(a: uintnat, b: uintnat, res: *mut uintnat) -> c_int {
    let (product, did_overflow) = a.overflowing_mul(b);
    *res = product;
    did_overflow as c_int
}

type asize_t = size_t;
type value = intnat;
type header_t = uintnat;
type mlsize_t = uintnat;
type tag_t = c_uint; // Actually, an unsigned char
type color_t = uintnat;

#[inline]
const fn Is_long(x: value) -> bool {
    x & 1 != 0
}
#[inline]
const fn Is_block(x: value) -> bool {
    x & 1 == 0
}

// Conversion macro names are always of the form  "to_from".
// Example: Val_long as in "Val from long" or "Val of long".

#[inline]
const fn Long_val(x: value) -> intnat {
    x >> 1
}
#[inline]
const fn Tag_hd(hd: header_t) -> tag_t {
    (hd & 0xFF) as tag_t
}
#[inline]
const fn Wosize_hd(hd: header_t) -> mlsize_t {
    hd >> 10
}
#[inline]
const unsafe fn Hd_val(v: value) -> header_t {
    *(v as *const header_t).offset(-1)
}
#[inline]
const unsafe fn Wosize_val(val: value) -> mlsize_t {
    Wosize_hd(Hd_val(val))
}
#[inline]
const fn Bsize_wsize(sz: mlsize_t) -> mlsize_t {
    sz.wrapping_mul(std::mem::size_of::<value>() as mlsize_t)
}
#[inline]
const unsafe fn Bosize_hd(hd: header_t) -> mlsize_t {
    Bsize_wsize(Wosize_hd(hd))
}
#[inline]
unsafe fn Tag_val(val: value) -> tag_t {
    *(val as *mut c_uchar).offset((std::mem::size_of::<value>() as c_ulong).wrapping_neg() as isize)
        as tag_t
}

/// Fields are numbered from 0.
#[inline]
const unsafe fn Field(x: value, i: usize) -> value {
    *(x as *const value).add(i)
}
#[inline]
unsafe fn Field_ptr_mut(x: value, i: usize) -> *mut value {
    &mut *(x as *mut value).add(i) as *mut value
}

// const No_scan_tag: c_uint = 251;
const Forward_tag: c_uint = 250;
const Infix_tag: c_uint = 249;
// const Object_tag: c_uint = 248;
const Closure_tag: c_uint = 247;
const Lazy_tag: c_uint = 246;
// const Cont_tag: c_uint = 245;
const Forcing_tag: c_uint = 244;
const Abstract_tag: c_uint = 251;
const String_tag: c_uint = 252;
const Double_tag: c_uint = 253;
const Double_array_tag: c_uint = 254;
const Custom_tag: c_uint = 255;

#[inline]
const unsafe fn Forward_val(v: value) -> value {
    Field(v, 0)
}
#[inline]
const unsafe fn Infix_offset_hd(hd: header_t) -> mlsize_t {
    Bosize_hd(hd)
}

/// Pointer to the first byte
#[inline]
const fn Bp_val(v: value) -> *const c_char {
    v as _
}

#[inline]
const fn String_val(v: value) -> *const c_char {
    Bp_val(v)
}

const Double_wosize: mlsize_t =
    (std::mem::size_of::<c_double>() / std::mem::size_of::<value>()) as mlsize_t;

#[inline]
const fn Make_header(wosize: mlsize_t, tag: tag_t, color: color_t) -> header_t {
    (wosize << 10)
        .wrapping_add(color as header_t)
        .wrapping_add(tag as header_t)
}

type caml_stat_block = *mut c_void;

// Flags affecting marshaling

const NO_SHARING: c_int = 1; // Flag to ignore sharing
const CLOSURES: c_int = 2; // Flag to allow marshaling code pointers
const COMPAT_32: c_int = 4; // Flag to ensure that output can safely be read back on a 32-bit platform

// Stack for pending values to marshal

const EXTERN_STACK_INIT_SIZE: usize = 256;
const EXTERN_STACK_MAX_SIZE: usize = 1024 * 1024 * 100;

#[derive(Copy, Clone)]
#[repr(C)]
struct extern_item {
    v: *mut value,
    count: mlsize_t,
}

// Hash table to record already-marshaled objects and their positions

#[derive(Copy, Clone)]
#[repr(C)]
struct object_position {
    obj: value,
    pos: uintnat,
}

// The hash table uses open addressing, linear probing, and a redundant
// representation:
// - a bitvector [present] records which entries of the table are occupied;
// - an array [entries] records (object, position) pairs for the entries
//     that are occupied.
// The bitvector is much smaller than the array (1/128th on 64-bit
// platforms, 1/64th on 32-bit platforms), so it has better locality,
// making it faster to determine that an object is not in the table.
// Also, it makes it faster to empty or initialize a table: only the
// [present] bitvector needs to be filled with zeros, the [entries]
// array can be left uninitialized.

#[derive(Copy, Clone)]
#[repr(C)]
struct position_table {
    shift: c_int,
    size: mlsize_t,                // size == 1 << (wordsize - shift)
    mask: mlsize_t,                // mask == size - 1
    threshold: mlsize_t,           // threshold == a fixed fraction of size
    present: *mut uintnat,         // [Bitvect_size(size)]
    entries: *mut object_position, // [size]
}

const Bits_word: usize = 8 * std::mem::size_of::<uintnat>();

#[inline]
const fn Bitvect_size(n: usize) -> usize {
    (n + Bits_word - 1) / Bits_word
}

const POS_TABLE_INIT_SIZE_LOG2: usize = 8;
const POS_TABLE_INIT_SIZE: usize = 1 << POS_TABLE_INIT_SIZE_LOG2;

// Multiplicative Fibonacci hashing
// (Knuth, TAOCP vol 3, section 6.4, page 518).
// HASH_FACTOR is (sqrt(5) - 1) / 2 * 2^wordsize.
const HASH_FACTOR: uintnat = 11400714819323198486;
#[inline]
const fn Hash(v: value, shift: c_int) -> mlsize_t {
    (v as uintnat).wrapping_mul(HASH_FACTOR) >> shift as uintnat
}

// When the table becomes 2/3 full, its size is increased.
#[inline]
const fn Threshold(sz: usize) -> usize {
    sz.wrapping_mul(2).wrapping_div(3)
}

// Accessing bitvectors

#[inline]
unsafe fn bitvect_test(bv: *mut uintnat, i: uintnat) -> uintnat {
    *bv.offset(i.wrapping_div(Bits_word as uintnat) as isize)
        & (1 << (i & (Bits_word - 1) as uintnat))
}

#[inline]
unsafe fn bitvect_set(bv: *mut uintnat, i: uintnat) {
    *bv.offset(i.wrapping_div(Bits_word as uintnat) as isize) |=
        1 << (i & (Bits_word - 1) as uintnat);
}

// Conversion to big-endian

#[inline]
fn store16(dst: &mut impl Write, n: c_int) {
    dst.write_all(&(n as i16).to_be_bytes()).unwrap()
}

#[inline]
fn store32(dst: &mut impl Write, n: intnat) {
    dst.write_all(&(n as i32).to_be_bytes()).unwrap()
}

#[inline]
fn store64(dst: &mut impl Write, n: int64_t) {
    dst.write_all(&(n as i64).to_be_bytes()).unwrap()
}

#[repr(C)]
struct State {
    extern_flags: c_int, // logical or of some of the flags

    obj_counter: uintnat, // Number of objects emitted so far
    size_32: uintnat,     // Size in words of 32-bit block for struct.
    size_64: uintnat,     // Size in words of 64-bit block for struct.

    // Stack for pending value to marshal
    extern_stack_init: [extern_item; EXTERN_STACK_INIT_SIZE],
    extern_stack: *mut extern_item,
    extern_stack_limit: *mut extern_item,

    // Hash table to record already marshalled objects
    pos_table_present_init: [uintnat; Bitvect_size(POS_TABLE_INIT_SIZE)],
    pos_table_entries_init: [object_position; POS_TABLE_INIT_SIZE],
    pos_table: position_table,

    // To buffer the output
    output: Vec<u8>,
}

impl State {
    unsafe fn new() -> *mut State {
        let mut extern_state: *mut State =
            caml_stat_alloc_noexc(std::mem::size_of::<State>() as asize_t) as _;
        if extern_state.is_null() {
            return std::ptr::null_mut();
        }

        (*extern_state).extern_flags = 0;
        (*extern_state).obj_counter = 0;
        (*extern_state).size_32 = 0;
        (*extern_state).size_64 = 0;
        (*extern_state).extern_stack = (*extern_state).extern_stack_init.as_mut_ptr();
        (*extern_state).extern_stack_limit =
            (*extern_state).extern_stack.add(EXTERN_STACK_INIT_SIZE);
        (*extern_state).output = vec![];

        extern_state
    }

    unsafe fn free(extern_state: *mut State) {
        if !extern_state.is_null() {
            drop(std::mem::take(&mut (*extern_state).output));
            caml_stat_free(extern_state as caml_stat_block);
        }
    }

    /// Free the extern stack if needed
    unsafe fn free_stack(&mut self) {
        if self.extern_stack != self.extern_stack_init.as_mut_ptr() {
            caml_stat_free(self.extern_stack as caml_stat_block);
            // Reinitialize the globals for next time around
            self.extern_stack = self.extern_stack_init.as_mut_ptr();
            self.extern_stack_limit = self.extern_stack.add(EXTERN_STACK_INIT_SIZE)
        };
    }

    unsafe fn resize_stack(&mut self, sp: *mut extern_item) -> *mut extern_item {
        let newsize: asize_t = (2 as asize_t).wrapping_mul(
            self.extern_stack_limit
                .wrapping_offset_from(self.extern_stack) as asize_t,
        );
        let sp_offset: asize_t = sp.wrapping_offset_from(self.extern_stack) as asize_t;

        if newsize >= EXTERN_STACK_MAX_SIZE as c_ulong {
            self.stack_overflow();
        }
        let newstack: *mut extern_item =
            caml_stat_calloc_noexc(newsize, std::mem::size_of::<extern_item>() as asize_t) as _;
        if newstack.is_null() {
            self.stack_overflow();
        }

        // Copy items from the old stack to the new stack
        memcpy(
            newstack as *mut c_void,
            self.extern_stack as *const c_void,
            (std::mem::size_of::<extern_item>() as c_ulong).wrapping_mul(sp_offset) as usize,
        );

        // Free the old stack if it is not the initial stack
        if self.extern_stack != self.extern_stack_init.as_mut_ptr() {
            caml_stat_free(self.extern_stack as caml_stat_block);
        }

        self.extern_stack = newstack;
        self.extern_stack_limit = newstack.offset(newsize as isize);
        newstack.offset(sp_offset as isize)
    }

    /// Initialize the position table
    unsafe fn init_position_table(&mut self) {
        if self.extern_flags & NO_SHARING != 0 {
            return;
        }
        self.pos_table.size = POS_TABLE_INIT_SIZE as mlsize_t;
        self.pos_table.shift = 8usize
            .wrapping_mul(std::mem::size_of::<value>())
            .wrapping_sub(POS_TABLE_INIT_SIZE_LOG2) as c_int;
        self.pos_table.mask = (POS_TABLE_INIT_SIZE - 1) as mlsize_t;
        self.pos_table.threshold = Threshold(POS_TABLE_INIT_SIZE) as mlsize_t;
        self.pos_table.present = self.pos_table_present_init.as_mut_ptr();
        self.pos_table.entries = self.pos_table_entries_init.as_mut_ptr();
        memset(
            self.pos_table_present_init.as_mut_ptr() as *mut c_void,
            0,
            Bitvect_size(POS_TABLE_INIT_SIZE).wrapping_mul(std::mem::size_of::<uintnat>()),
        );
    }

    /// Free the position table
    unsafe fn free_position_table(&mut self) {
        if self.extern_flags & NO_SHARING != 0 {
            return;
        }
        if self.pos_table.present != self.pos_table_present_init.as_mut_ptr() {
            caml_stat_free(self.pos_table.present as caml_stat_block);
            caml_stat_free(self.pos_table.entries as caml_stat_block);
            // Protect against repeated calls to free_position_table
            self.pos_table.present = self.pos_table_present_init.as_mut_ptr();
            self.pos_table.entries = self.pos_table_entries_init.as_mut_ptr()
        };
    }

    /// Grow the position table
    unsafe fn resize_position_table(&mut self) {
        let new_size: mlsize_t;
        let mut new_byte_size: mlsize_t = 0;
        let new_shift: c_int;
        let new_present: *mut uintnat;
        let new_entries: *mut object_position;
        let mut i: uintnat;
        let mut h: uintnat;
        let old: position_table = self.pos_table;

        // Grow the table quickly (x 8) up to 10^6 entries,
        // more slowly (x 2) afterwards.
        if old.size < 1000000 {
            new_size = (old.size).wrapping_mul(8);
            new_shift = old.shift - 3;
        } else {
            new_size = (old.size).wrapping_mul(2);
            new_shift = old.shift - 1;
        }
        if new_size == 0
            || caml_umul_overflow(
                new_size,
                std::mem::size_of::<object_position>() as c_ulong,
                &mut new_byte_size,
            ) != 0
        {
            self.out_of_memory();
        }
        new_entries = caml_stat_alloc_noexc(new_byte_size) as _;
        if new_entries.is_null() {
            self.out_of_memory();
        }
        new_present = caml_stat_calloc_noexc(
            Bitvect_size(new_size as usize) as asize_t,
            std::mem::size_of::<uintnat>() as asize_t,
        ) as _;
        if new_present.is_null() {
            caml_stat_free(new_entries as caml_stat_block);
            self.out_of_memory();
        }
        self.pos_table.size = new_size;
        self.pos_table.shift = new_shift;
        self.pos_table.mask = new_size.wrapping_sub(1);
        self.pos_table.threshold = Threshold(new_size as usize) as mlsize_t;
        self.pos_table.present = new_present;
        self.pos_table.entries = new_entries;

        // Insert every entry of the old table in the new table
        i = 0;
        while i < old.size {
            if bitvect_test(old.present, i) != 0 {
                h = Hash((*old.entries.offset(i as isize)).obj, self.pos_table.shift);
                while bitvect_test(new_present, h) != 0 {
                    h = h.wrapping_add(1) & self.pos_table.mask
                }
                bitvect_set(new_present, h);
                *new_entries.offset(h as isize) = *old.entries.offset(i as isize)
            }
            i = i.wrapping_add(1)
        }

        // Free the old tables if they are not the initial ones
        if old.present != self.pos_table_present_init.as_mut_ptr() {
            caml_stat_free(old.present as caml_stat_block);
            caml_stat_free(old.entries as caml_stat_block);
        }
    }

    /// Determine whether the given object [obj] is in the hash table.
    /// If so, set `*pos_out` to its position in the output and return 1.
    /// If not, set `*h_out` to the hash value appropriate for
    /// `record_location` and return 0.
    #[inline]
    unsafe fn lookup_position(
        &mut self,
        obj: value,
        pos_out: *mut uintnat,
        h_out: *mut uintnat,
    ) -> c_int {
        let mut h: uintnat = Hash(obj, self.pos_table.shift);
        loop {
            if bitvect_test(self.pos_table.present, h) == 0 {
                *h_out = h;
                return 0;
            }
            if (*self.pos_table.entries.offset(h as isize)).obj == obj {
                *pos_out = (*self.pos_table.entries.offset(h as isize)).pos;
                return 1;
            }
            h = h.wrapping_add(1) & self.pos_table.mask
        }
    }

    /// Record the output position for the given object [obj].
    ///
    /// The [h] parameter is the index in the hash table where the object
    /// must be inserted.  It was determined during lookup.
    unsafe fn record_location(&mut self, obj: value, h: uintnat) {
        if self.extern_flags & NO_SHARING != 0 {
            return;
        }
        bitvect_set(self.pos_table.present, h);
        (*self.pos_table.entries.offset(h as isize)).obj = obj;
        (*self.pos_table.entries.offset(h as isize)).pos = self.obj_counter;
        self.obj_counter = self.obj_counter.wrapping_add(1);
        if self.obj_counter >= self.pos_table.threshold {
            self.resize_position_table();
        };
    }

    // To buffer the output

    fn init_output(&mut self) {
        self.output = Vec::with_capacity(SIZE_EXTERN_OUTPUT_BLOCK);
    }

    fn close_output(&mut self) {}

    unsafe fn free_output(&mut self) {
        drop(std::mem::take(&mut self.output));
        self.free_stack();
        self.free_position_table();
    }

    unsafe fn output_length(&mut self) -> intnat {
        self.output.len() as intnat
    }

    // Exception raising, with cleanup

    unsafe fn out_of_memory(&mut self) -> ! {
        self.free_output();
        caml_raise_out_of_memory();
    }

    unsafe fn invalid_argument(&mut self, msg: *const c_char) -> ! {
        self.free_output();
        caml_invalid_argument(msg);
    }

    unsafe fn failwith(&mut self, msg: *const c_char) -> ! {
        self.free_output();
        caml_failwith(msg);
    }

    unsafe fn stack_overflow(&mut self) -> ! {
        caml_gc_message(
            0x4,
            b"Stack overflow in marshaling value\n\x00" as *const u8 as *const c_char,
        );
        self.free_output();
        caml_raise_out_of_memory();
    }

    // Write characters, integers, and blocks in the output buffer

    #[inline]
    fn write(&mut self, c: c_int) {
        self.output.write_all(&[c as u8]).unwrap();
    }

    unsafe fn writeblock(&mut self, data: *const c_char, len: intnat) {
        self.output
            .write_all(std::slice::from_raw_parts(data as *const u8, len as usize))
            .unwrap();
    }

    #[inline]
    unsafe fn writeblock_float8(&mut self, data: *const c_double, ndoubles: intnat) {
        if ARCH_FLOAT_ENDIANNESS == 0x01234567 || ARCH_FLOAT_ENDIANNESS == 0x76543210 {
            self.writeblock(data as *const c_char, ndoubles * 8);
        } else {
            unimplemented!()
        }
    }

    fn writecode8(&mut self, code: c_int, val: intnat) {
        self.output.write_all(&[code as u8, val as u8]).unwrap();
    }

    fn writecode16(&mut self, code: c_int, val: intnat) {
        self.output.write_all(&[code as u8]).unwrap();
        store16(&mut self.output, val as c_int);
    }

    fn writecode32(&mut self, code: c_int, val: intnat) {
        self.output.write_all(&[code as u8]).unwrap();
        store32(&mut self.output, val);
    }

    fn writecode64(&mut self, code: c_int, val: intnat) {
        self.output.write_all(&[code as u8]).unwrap();
        store64(&mut self.output, val);
    }

    /// Marshaling integers
    #[inline]
    unsafe fn extern_int(&mut self, n: intnat) {
        if (0..0x40).contains(&n) {
            self.write(PREFIX_SMALL_INT + n as c_int);
        } else if (-(1 << 7)..(1 << 7)).contains(&n) {
            self.writecode8(CODE_INT8, n);
        } else if (-(1 << 15)..(1 << 15)).contains(&n) {
            self.writecode16(CODE_INT16, n);
        } else if !(-(1 << 30)..(1 << 30)).contains(&n) {
            if self.extern_flags & COMPAT_32 != 0 {
                self.failwith(
                    b"output_value: integer cannot be read back on 32-bit platform\x00" as *const u8
                        as *const c_char,
                );
            }
            self.writecode64(CODE_INT64, n);
        } else {
            self.writecode32(CODE_INT32, n);
        };
    }

    /// Marshaling references to previously-marshaled blocks
    #[inline]
    unsafe fn extern_shared_reference(&mut self, d: uintnat) {
        if d < 0x100 {
            self.writecode8(CODE_SHARED8, d as intnat);
        } else if d < 0x10000 {
            self.writecode16(CODE_SHARED16, d as intnat);
        } else if d >= 1 << 32 {
            self.writecode64(CODE_SHARED64, d as intnat);
        } else {
            self.writecode32(CODE_SHARED32, d as intnat);
        };
    }

    /// Marshaling block headers
    #[inline]
    unsafe fn extern_header(&mut self, sz: mlsize_t, tag: tag_t) {
        if tag < 16 && sz < 8 {
            self.write(
                PREFIX_SMALL_BLOCK
                    .wrapping_add(tag as c_int)
                    .wrapping_add((sz << 4) as c_int),
            );
        } else {
            // Note: ocaml-14.4.0 uses `Caml_white` (`0 << 8`)
            // ('caml/runtime/gc.h'). That's why we use this here, so that we
            // may test what this program generates vs. ocaml-4.14.0 in use
            // today.
            //
            // In PR https://github.com/ocaml/ocaml/pull/10831, in commit
            // `https://github.com/ocaml/ocaml/commit/868265f4532a2cc33bbffd83221c9613e743d759`
            // this becomes,
            //   let hd: header_t = Make_header(sz, tag, NOT_MARKABLE);
            // where, `NOT_MARKABLE` (`3 << 8`) ('caml/runtime/shared_heap.h').
            let hd: header_t = Make_header(sz, tag, 0 << 8);

            if sz > 0x3FFFFF && self.extern_flags & COMPAT_32 != 0 {
                self.failwith(
                    b"output_value: array cannot be read back on 32-bit platform\x00" as *const u8
                        as *const c_char,
                );
            }
            if hd < 1 << 32 {
                self.writecode32(CODE_BLOCK32, hd as intnat);
            } else {
                self.writecode64(CODE_BLOCK64, hd as intnat);
            }
        };
    }

    #[inline]
    unsafe fn extern_string(&mut self, v: value, len: mlsize_t) {
        if len < 0x20 {
            self.write(PREFIX_SMALL_STRING.wrapping_add(len as c_int));
        } else if len < 0x100 {
            self.writecode8(CODE_STRING8, len as intnat);
        } else {
            if len > 0xFFFFFB && self.extern_flags & COMPAT_32 != 0 {
                self.failwith(
                    b"output_value: string cannot be read back on 32-bit platform\x00" as *const u8
                        as *const c_char,
                );
            }
            if len < 1 << 32 {
                self.writecode32(CODE_STRING32, len as intnat);
            } else {
                self.writecode64(CODE_STRING64, len as intnat);
            }
        }
        self.writeblock(String_val(v), len as intnat);
    }

    /// Marshaling FP numbers
    #[inline]
    unsafe fn extern_double(&mut self, v: value) {
        self.write(CODE_DOUBLE_NATIVE);
        self.writeblock_float8(v as *mut c_double, 1 as intnat);
    }

    /// Marshaling FP arrays
    #[inline]
    unsafe fn extern_double_array(&mut self, v: value, nfloats: mlsize_t) {
        if nfloats < 0x100 {
            self.writecode8(CODE_DOUBLE_ARRAY8_NATIVE, nfloats as intnat);
        } else {
            if nfloats > 0x1FFFFF && self.extern_flags & COMPAT_32 != 0 {
                self.failwith(
                    b"output_value: float array cannot be read back on 32-bit platform\x00"
                        as *const u8 as *const c_char,
                );
            }
            if nfloats < 1 << 32 {
                self.writecode32(CODE_DOUBLE_ARRAY32_NATIVE, nfloats as intnat);
            } else {
                self.writecode64(CODE_DOUBLE_ARRAY64_NATIVE, nfloats as intnat);
            }
        }
        self.writeblock_float8(v as *mut c_double, nfloats as intnat);
    }

    /// Marshal the given value in the output buffer
    unsafe fn extern_rec(&mut self, mut v: value) {
        let mut goto_next_item: bool;

        let mut sp: *mut extern_item;
        let mut h: uintnat = 0;
        let mut pos: uintnat = 0;

        self.init_position_table();
        sp = self.extern_stack;

        loop {
            if Is_long(v) {
                self.extern_int(Long_val(v));
            } else {
                let hd: header_t = Hd_val(v);
                let tag: tag_t = Tag_hd(hd);
                let sz: mlsize_t = Wosize_hd(hd);

                if tag == Forward_tag {
                    let f: value = Forward_val(v);
                    if Is_block(f)
                        && (Tag_val(f) == Forward_tag
                            || Tag_val(f) == Lazy_tag
                            || Tag_val(f) == Forcing_tag
                            || Tag_val(f) == Double_tag)
                    {
                        // Do not short-circuit the pointer.
                    } else {
                        v = f;
                        continue;
                    }
                }
                // Atoms are treated specially for two reasons: they are not allocated
                // in the externed block, and they are automatically shared.
                if sz == 0 {
                    self.extern_header(0, tag);
                } else {
                    // Check if object already seen
                    if self.extern_flags & NO_SHARING == 0 {
                        if self.lookup_position(v, &mut pos, &mut h) != 0 {
                            self.extern_shared_reference(self.obj_counter.wrapping_sub(pos));
                            goto_next_item = true;
                        } else {
                            goto_next_item = false;
                        }
                    } else {
                        goto_next_item = false;
                    }
                    if !goto_next_item {
                        // Output the contents of the object
                        match tag {
                            String_tag => {
                                let len: mlsize_t = caml_string_length(v);
                                self.extern_string(v, len);
                                self.size_32 = self.size_32.wrapping_add(
                                    (1 as uintnat)
                                        .wrapping_add(len.wrapping_add(4).wrapping_div(4)),
                                );
                                self.size_64 = self.size_64.wrapping_add(
                                    (1 as uintnat)
                                        .wrapping_add(len.wrapping_add(8).wrapping_div(8)),
                                );
                                self.record_location(v, h);
                            }
                            Double_tag => {
                                self.extern_double(v);
                                self.size_32 = self.size_32.wrapping_add(1 + 2);
                                self.size_64 = self.size_64.wrapping_add(1 + 1);
                                self.record_location(v, h);
                            }
                            Double_array_tag => {
                                let nfloats: mlsize_t = Wosize_val(v) / Double_wosize;
                                self.extern_double_array(v, nfloats);
                                self.size_32 = (*self).size_32.wrapping_add(
                                    (1 as uintnat).wrapping_add(nfloats.wrapping_mul(2)),
                                );
                                self.size_64 = (*self)
                                    .size_64
                                    .wrapping_add((1 as uintnat).wrapping_add(nfloats));
                                self.record_location(v, h);
                            }
                            Abstract_tag => {
                                self.invalid_argument(
                                    b"output_value: abstract value (Abstract)\x00" as *const u8
                                        as *const c_char,
                                );
                            }
                            Infix_tag => {
                                self.writecode32(CODE_INFIXPOINTER, Infix_offset_hd(hd) as intnat);
                                v = (v as uintnat).wrapping_sub(Infix_offset_hd(hd)) as value; // PR#5772
                                continue;
                            }
                            Custom_tag => self.invalid_argument(
                                b"output_value: marshaling of custom blocks not implemented\0"
                                    as *const u8 as *const c_char,
                            ),
                            Closure_tag => self.invalid_argument(
                                b"output_value: marshaling of closures not implemented\0"
                                    as *const u8 as *const c_char,
                            ),
                            _ => {
                                self.extern_header(sz, tag);
                                self.size_32 =
                                    self.size_32.wrapping_add((1 as uintnat).wrapping_add(sz));
                                self.size_64 =
                                    self.size_64.wrapping_add((1 as uintnat).wrapping_add(sz));
                                self.record_location(v, h);
                                // Remember that we still have to serialize fields 1 ... sz - 1
                                if sz > 1 {
                                    sp = sp.offset(1);
                                    if sp >= self.extern_stack_limit {
                                        sp = self.resize_stack(sp)
                                    }
                                    (*sp).v = Field_ptr_mut(v, 1);
                                    (*sp).count = sz.wrapping_sub(1)
                                }
                                // Continue serialization with the first field
                                v = *(v as *mut value).offset(0);
                                continue;
                            }
                        }
                    }
                }
            }
            // C goto label `next_item:` here

            // Pop one more item to marshal, if any
            if sp == self.extern_stack {
                // We are done.   Cleanup the stack and leave the function
                self.free_stack();
                self.free_position_table();
                return;
            }
            let fresh8 = (*sp).v;
            (*sp).v = (*sp).v.offset(1);
            v = *fresh8;
            (*sp).count = (*sp).count.wrapping_sub(1);
            if (*sp).count == 0 {
                sp = sp.offset(-1)
            }
        }
        // Never reached as function leaves with return
    }

    unsafe fn extern_value(
        &mut self,
        v: value,
        flags: value,
        mut header: &mut [u8],  // out
        header_len: &mut usize, // out
    ) -> intnat {
        static EXTERN_FLAG_VALUES: [c_int; 3] = [NO_SHARING, CLOSURES, COMPAT_32];

        let res_len: intnat;
        // Parse flag list
        self.extern_flags = caml_convert_flag_list(flags, EXTERN_FLAG_VALUES.as_ptr());
        // Initializations
        self.obj_counter = 0;
        self.size_32 = 0;
        self.size_64 = 0;
        // Marshal the object
        self.extern_rec(v);
        // Record end of output
        self.close_output();
        // Write the header
        res_len = self.output_length();
        if res_len >= (1 << 32) || self.size_32 >= (1 << 32) || self.size_64 >= (1 << 32) {
            // The object is too big for the small header format.
            // Fail if we are in compat32 mode, or use big header.
            if self.extern_flags & COMPAT_32 != 0 {
                self.free_output();
                caml_failwith(
                    b"output_value: object too big to be read back on 32-bit platform\x00"
                        as *const u8 as *const c_char,
                );
            }
            store32(&mut header, MAGIC_NUMBER_BIG as intnat);
            store32(&mut header, 0);
            store64(&mut header, res_len);
            store64(&mut header, self.obj_counter as int64_t);
            store64(&mut header, self.size_64 as int64_t);
            *header_len = 32;
            return res_len;
        }
        // Use the small header format
        store32(&mut header, MAGIC_NUMBER_SMALL as intnat);
        store32(&mut header, res_len);
        store32(&mut header, self.obj_counter as intnat);
        store32(&mut header, self.size_32 as intnat);
        store32(&mut header, self.size_64 as intnat);
        *header_len = 20;
        res_len
    }
}

unsafe fn output_val<W: std::io::Write>(w: &mut W, v: value, flags: value) -> std::io::Result<()> {
    let mut header: [u8; 32] = [0; 32];
    let mut header_len = 0;
    let s: &mut State = State::new().as_mut().expect("nonnull");
    s.init_output();
    s.extern_value(v, flags, &mut header, &mut header_len);
    let output = std::mem::take(&mut s.output);
    State::free(s as *mut State);
    w.write_all(&header[0..header_len])?;
    w.write_all(&output)?;
    w.flush()
}

#[no_mangle]
pub unsafe extern "C" fn ocamlrep_marshal_output_value_to_string(v: value, flags: value) -> value {
    let mut vec = vec![];
    output_val(&mut vec, v, flags).unwrap();
    let res: value = caml_alloc_string(vec.len() as mlsize_t);
    memcpy(res as *mut c_void, vec.as_ptr() as *const c_void, vec.len());
    res
}
