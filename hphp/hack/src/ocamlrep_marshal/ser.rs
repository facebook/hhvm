#![allow(non_camel_case_types, non_snake_case, non_upper_case_globals)]
#![allow(clippy::needless_late_init)]

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
use libc::strlen;

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
    fn caml_find_code_fragment_by_pc(pc: *mut c_char) -> *mut code_fragment;
    fn caml_convert_flag_list(_: value, _: *const c_int) -> c_int;
    fn caml_digest_of_code_fragment(_: *mut code_fragment) -> *mut c_uchar;
    fn caml_stat_alloc_noexc(_: asize_t) -> caml_stat_block;
    fn caml_failwith(msg: *const c_char) -> !;
    fn caml_invalid_argument(msg: *const c_char) -> !;
    fn caml_raise_out_of_memory() -> !;
    fn caml_stat_calloc_noexc(_: asize_t, _: asize_t) -> caml_stat_block;
    fn caml_stat_free(_: caml_stat_block);
    fn caml_fatal_error(_: *const c_char, _: ...) -> !;
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

/// Arity and start env
#[inline]
const unsafe fn Closinfo_val(val: value) -> value {
    Field(val, 1)
}
#[inline]
const fn Arity_closinfo(info: value) -> intnat {
    (info as intnat) >> 56
}
#[inline]
const fn Start_env_closinfo(info: value) -> uintnat {
    ((info as uintnat) << 8) >> 9
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

const NOT_MARKABLE: color_t = 3 << 8;

#[inline]
const fn Make_header(wosize: mlsize_t, tag: tag_t, color: color_t) -> header_t {
    (wosize << 10)
        .wrapping_add(color as header_t)
        .wrapping_add(tag as header_t)
}

#[derive(Copy, Clone)]
#[repr(C)]
struct custom_fixed_length {
    bsize_32: intnat,
    bsize_64: intnat,
}
type digest_status = c_uint;

#[derive(Copy, Clone)]
#[repr(C)]
struct code_fragment {
    code_start: *mut c_char,
    code_end: *mut c_char,
    fragnum: c_int,
    digest: [c_uchar; 16],
    digest_status: digest_status,
}

type caml_stat_block = *mut c_void;

#[derive(Copy, Clone)]
#[repr(C)]
struct custom_operations {
    identifier: *const c_char,
    finalize: Option<unsafe extern "C" fn(_: value) -> ()>,
    compare: Option<unsafe extern "C" fn(_: value, _: value) -> c_int>,
    hash: Option<unsafe extern "C" fn(_: value) -> intnat>,
    serialize: Option<unsafe extern "C" fn(_: value, _: *mut uintnat, _: *mut uintnat) -> ()>,
    deserialize: Option<unsafe extern "C" fn(_: *mut c_void) -> uintnat>,
    compare_ext: Option<unsafe extern "C" fn(_: value, _: value) -> c_int>,
    fixed_length: *const custom_fixed_length,
}

#[inline]
const unsafe fn Custom_ops_val(v: value) -> *const custom_operations {
    *(v as *const *const custom_operations)
}

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

#[derive(Copy, Clone)]
#[repr(C)]
struct output_block {
    next: *mut output_block,
    end: *mut c_char,
    data: *mut [c_char],
}

#[derive(Copy, Clone)]
#[repr(C)]
struct caml_extern_state {
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
    extern_userprovided_output: *mut c_char,
    extern_ptr: *mut c_char,
    extern_limit: *mut c_char,

    extern_output_first: *mut output_block,
    extern_output_block: *mut output_block,
}

unsafe fn new_extern_state() -> *mut caml_extern_state {
    let mut extern_state: *mut caml_extern_state =
        caml_stat_alloc_noexc(std::mem::size_of::<caml_extern_state>() as asize_t) as _;
    if extern_state.is_null() {
        return std::ptr::null_mut();
    }

    (*extern_state).extern_flags = 0;
    (*extern_state).obj_counter = 0;
    (*extern_state).size_32 = 0;
    (*extern_state).size_64 = 0;
    (*extern_state).extern_stack = (*extern_state).extern_stack_init.as_mut_ptr();
    (*extern_state).extern_stack_limit = (*extern_state).extern_stack.add(EXTERN_STACK_INIT_SIZE);

    extern_state
}

unsafe fn free_extern_state(extern_state: *mut caml_extern_state) {
    if !extern_state.is_null() {
        caml_stat_free(extern_state as caml_stat_block);
    }
}

/// Free the extern stack if needed
unsafe fn extern_free_stack(s: &mut caml_extern_state) {
    if s.extern_stack != s.extern_stack_init.as_mut_ptr() {
        caml_stat_free(s.extern_stack as caml_stat_block);
        // Reinitialize the globals for next time around
        s.extern_stack = s.extern_stack_init.as_mut_ptr();
        s.extern_stack_limit = s.extern_stack.add(EXTERN_STACK_INIT_SIZE)
    };
}

unsafe fn extern_resize_stack(s: &mut caml_extern_state, sp: *mut extern_item) -> *mut extern_item {
    let newsize: asize_t = (2 as asize_t)
        .wrapping_mul(s.extern_stack_limit.wrapping_offset_from(s.extern_stack) as asize_t);
    let sp_offset: asize_t = sp.wrapping_offset_from(s.extern_stack) as asize_t;

    if newsize >= EXTERN_STACK_MAX_SIZE as c_ulong {
        extern_stack_overflow(s);
    }
    let newstack: *mut extern_item =
        caml_stat_calloc_noexc(newsize, std::mem::size_of::<extern_item>() as asize_t) as _;
    if newstack.is_null() {
        extern_stack_overflow(s);
    }

    // Copy items from the old stack to the new stack
    memcpy(
        newstack as *mut c_void,
        s.extern_stack as *const c_void,
        (std::mem::size_of::<extern_item>() as c_ulong).wrapping_mul(sp_offset) as usize,
    );

    // Free the old stack if it is not the initial stack
    if s.extern_stack != s.extern_stack_init.as_mut_ptr() {
        caml_stat_free(s.extern_stack as caml_stat_block);
    }

    s.extern_stack = newstack;
    s.extern_stack_limit = newstack.offset(newsize as isize);
    newstack.offset(sp_offset as isize)
}

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

/// Initialize the position table
unsafe fn extern_init_position_table(s: &mut caml_extern_state) {
    if s.extern_flags & NO_SHARING != 0 {
        return;
    }
    s.pos_table.size = POS_TABLE_INIT_SIZE as mlsize_t;
    s.pos_table.shift = 8usize
        .wrapping_mul(std::mem::size_of::<value>())
        .wrapping_sub(POS_TABLE_INIT_SIZE_LOG2) as c_int;
    s.pos_table.mask = (POS_TABLE_INIT_SIZE - 1) as mlsize_t;
    s.pos_table.threshold = Threshold(POS_TABLE_INIT_SIZE) as mlsize_t;
    s.pos_table.present = s.pos_table_present_init.as_mut_ptr();
    s.pos_table.entries = s.pos_table_entries_init.as_mut_ptr();
    memset(
        s.pos_table_present_init.as_mut_ptr() as *mut c_void,
        0,
        Bitvect_size(POS_TABLE_INIT_SIZE).wrapping_mul(std::mem::size_of::<uintnat>()),
    );
}

/// Free the position table
unsafe fn extern_free_position_table(s: &mut caml_extern_state) {
    if s.extern_flags & NO_SHARING != 0 {
        return;
    }
    if s.pos_table.present != s.pos_table_present_init.as_mut_ptr() {
        caml_stat_free(s.pos_table.present as caml_stat_block);
        caml_stat_free(s.pos_table.entries as caml_stat_block);
        // Protect against repeated calls to extern_free_position_table
        s.pos_table.present = s.pos_table_present_init.as_mut_ptr();
        s.pos_table.entries = s.pos_table_entries_init.as_mut_ptr()
    };
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

/// Grow the position table
unsafe fn extern_resize_position_table(s: &mut caml_extern_state) {
    let new_size: mlsize_t;
    let mut new_byte_size: mlsize_t = 0;
    let new_shift: c_int;
    let new_present: *mut uintnat;
    let new_entries: *mut object_position;
    let mut i: uintnat;
    let mut h: uintnat;
    let old: position_table = s.pos_table;

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
        extern_out_of_memory(s);
    }
    new_entries = caml_stat_alloc_noexc(new_byte_size) as _;
    if new_entries.is_null() {
        extern_out_of_memory(s);
    }
    new_present = caml_stat_calloc_noexc(
        Bitvect_size(new_size as usize) as asize_t,
        std::mem::size_of::<uintnat>() as asize_t,
    ) as _;
    if new_present.is_null() {
        caml_stat_free(new_entries as caml_stat_block);
        extern_out_of_memory(s);
    }
    s.pos_table.size = new_size;
    s.pos_table.shift = new_shift;
    s.pos_table.mask = new_size.wrapping_sub(1);
    s.pos_table.threshold = Threshold(new_size as usize) as mlsize_t;
    s.pos_table.present = new_present;
    s.pos_table.entries = new_entries;

    // Insert every entry of the old table in the new table
    i = 0;
    while i < old.size {
        if bitvect_test(old.present, i) != 0 {
            h = Hash((*old.entries.offset(i as isize)).obj, s.pos_table.shift);
            while bitvect_test(new_present, h) != 0 {
                h = h.wrapping_add(1) & s.pos_table.mask
            }
            bitvect_set(new_present, h);
            *new_entries.offset(h as isize) = *old.entries.offset(i as isize)
        }
        i = i.wrapping_add(1)
    }

    // Free the old tables if they are not the initial ones
    if old.present != s.pos_table_present_init.as_mut_ptr() {
        caml_stat_free(old.present as caml_stat_block);
        caml_stat_free(old.entries as caml_stat_block);
    }
}

/// Determine whether the given object [obj] is in the hash table.
/// If so, set `*pos_out` to its position in the output and return 1.
/// If not, set `*h_out` to the hash value appropriate for
/// `extern_record_location` and return 0.
#[inline]
unsafe fn extern_lookup_position(
    s: &mut caml_extern_state,
    obj: value,
    pos_out: *mut uintnat,
    h_out: *mut uintnat,
) -> c_int {
    let mut h: uintnat = Hash(obj, s.pos_table.shift);
    loop {
        if bitvect_test(s.pos_table.present, h) == 0 {
            *h_out = h;
            return 0;
        }
        if (*s.pos_table.entries.offset(h as isize)).obj == obj {
            *pos_out = (*s.pos_table.entries.offset(h as isize)).pos;
            return 1;
        }
        h = h.wrapping_add(1) & s.pos_table.mask
    }
}

/// Record the output position for the given object [obj].
///
/// The [h] parameter is the index in the hash table where the object
/// must be inserted.  It was determined during lookup.
unsafe fn extern_record_location(s: &mut caml_extern_state, obj: value, h: uintnat) {
    if s.extern_flags & NO_SHARING != 0 {
        return;
    }
    bitvect_set(s.pos_table.present, h);
    (*s.pos_table.entries.offset(h as isize)).obj = obj;
    (*s.pos_table.entries.offset(h as isize)).pos = s.obj_counter;
    s.obj_counter = s.obj_counter.wrapping_add(1);
    if s.obj_counter >= s.pos_table.threshold {
        extern_resize_position_table(s);
    };
}

// To buffer the output

unsafe fn init_extern_output(s: &mut caml_extern_state) {
    s.extern_userprovided_output = std::ptr::null_mut();
    s.extern_output_first =
        caml_stat_alloc_noexc(std::mem::size_of::<output_block>() as c_ulong) as _;
    if s.extern_output_first.is_null() {
        caml_raise_out_of_memory();
    }
    let sz = SIZE_EXTERN_OUTPUT_BLOCK;
    (*s.extern_output_first).data =
        std::slice::from_raw_parts_mut(caml_stat_alloc_noexc(sz as c_ulong) as *mut c_char, sz);
    if (*s.extern_output_first).data.is_null() {
        caml_raise_out_of_memory();
    }
    s.extern_output_block = s.extern_output_first;
    (*s.extern_output_block).next = std::ptr::null_mut();
    s.extern_ptr = (*s.extern_output_block).data.as_mut_ptr();
    s.extern_limit = (*s.extern_output_block).data.as_mut_ptr().add(sz);
}

unsafe fn close_extern_output(s: &mut caml_extern_state) {
    if s.extern_userprovided_output.is_null() {
        (*s.extern_output_block).end = s.extern_ptr
    };
}

unsafe fn free_extern_output(s: &mut caml_extern_state) {
    let mut blk: *mut output_block;
    let mut nextblk: *mut output_block;

    if s.extern_userprovided_output.is_null() {
        blk = s.extern_output_first;
        while !blk.is_null() {
            nextblk = (*blk).next;
            caml_stat_free((*blk).data as caml_stat_block);
            caml_stat_free(blk as caml_stat_block);
            blk = nextblk
        }
        s.extern_output_first = std::ptr::null_mut()
    }
    extern_free_stack(s);
    extern_free_position_table(s);
}

unsafe fn grow_extern_output(s: &mut caml_extern_state, required: intnat) {
    let blk: *mut output_block;
    let extra: intnat;

    if !s.extern_userprovided_output.is_null() {
        extern_failwith(
            s,
            b"Marshal.to_buffer: buffer overflow\x00" as *const u8 as *const c_char,
        );
    }
    (*s.extern_output_block).end = s.extern_ptr;
    if required <= (SIZE_EXTERN_OUTPUT_BLOCK / 2) as intnat {
        extra = 0
    } else {
        extra = required
    };
    blk = caml_stat_alloc_noexc(std::mem::size_of::<output_block>() as c_ulong) as _;
    if blk.is_null() {
        extern_out_of_memory(s);
    }
    let sz = SIZE_EXTERN_OUTPUT_BLOCK.wrapping_add(extra as usize);
    (*blk).data =
        std::slice::from_raw_parts_mut(caml_stat_alloc_noexc(sz as c_ulong) as *mut c_char, sz);
    if (*blk).data.is_null() {
        extern_out_of_memory(s);
    }
    (*s.extern_output_block).next = blk;
    s.extern_output_block = blk;
    (*s.extern_output_block).next = std::ptr::null_mut();
    s.extern_ptr = (*s.extern_output_block).data.as_mut_ptr();
    s.extern_limit = (*s.extern_output_block).data.as_mut_ptr().add(sz);
}

unsafe fn extern_output_length(s: &mut caml_extern_state) -> intnat {
    let mut blk: *mut output_block;
    let mut len: intnat;

    if !s.extern_userprovided_output.is_null() {
        s.extern_ptr
            .wrapping_offset_from(s.extern_userprovided_output) as intnat
    } else {
        len = 0;
        blk = s.extern_output_first;
        while !blk.is_null() {
            len += (*blk).end.wrapping_offset_from((*blk).data.as_mut_ptr()) as intnat;
            blk = (*blk).next
        }
        len
    }
}

// Exception raising, with cleanup

unsafe fn extern_out_of_memory(s: &mut caml_extern_state) -> ! {
    free_extern_output(s);
    caml_raise_out_of_memory();
}

unsafe fn extern_invalid_argument(s: &mut caml_extern_state, msg: *const c_char) -> ! {
    free_extern_output(s);
    caml_invalid_argument(msg);
}

unsafe fn extern_failwith(s: &mut caml_extern_state, msg: *const c_char) -> ! {
    free_extern_output(s);
    caml_failwith(msg);
}

unsafe fn extern_stack_overflow(s: &mut caml_extern_state) -> ! {
    caml_gc_message(
        0x4,
        b"Stack overflow in marshaling value\n\x00" as *const u8 as *const c_char,
    );
    free_extern_output(s);
    caml_raise_out_of_memory();
}

// Conversion to big-endian

#[inline]
unsafe fn store16(dst: *mut c_char, n: c_int) {
    *dst.offset(0) = (n >> 8) as c_char;
    *dst.offset(1) = n as c_char;
}

#[inline]
unsafe fn store32(dst: *mut c_char, n: intnat) {
    *dst.offset(0) = (n >> 24) as c_char;
    *dst.offset(1) = (n >> 16) as c_char;
    *dst.offset(2) = (n >> 8) as c_char;
    *dst.offset(3) = n as c_char;
}

#[inline]
unsafe fn store64(dst: *mut c_char, n: int64_t) {
    *dst.offset(0) = (n >> 56) as c_char;
    *dst.offset(1) = (n >> 48) as c_char;
    *dst.offset(2) = (n >> 40) as c_char;
    *dst.offset(3) = (n >> 32) as c_char;
    *dst.offset(4) = (n >> 24) as c_char;
    *dst.offset(5) = (n >> 16) as c_char;
    *dst.offset(6) = (n >> 8) as c_char;
    *dst.offset(7) = n as c_char;
}

// Write characters, integers, and blocks in the output buffer

#[inline]
unsafe fn write(s: &mut caml_extern_state, c: c_int) {
    if s.extern_ptr >= s.extern_limit {
        grow_extern_output(s, 1);
    }
    let fresh3 = s.extern_ptr;
    s.extern_ptr = s.extern_ptr.offset(1);
    *fresh3 = c as c_char;
}

unsafe fn writeblock(s: &mut caml_extern_state, data: *const c_char, len: intnat) {
    if s.extern_ptr.offset(len as isize) > s.extern_limit {
        grow_extern_output(s, len);
    }
    memcpy(
        s.extern_ptr as *mut c_void,
        data as *const c_void,
        len as usize,
    );
    s.extern_ptr = s.extern_ptr.offset(len as isize);
}

#[inline]
unsafe fn writeblock_float8(s: &mut caml_extern_state, data: *const c_double, ndoubles: intnat) {
    writeblock(s, data as *const c_char, ndoubles * 8);
}

unsafe fn writecode8(s: &mut caml_extern_state, code: c_int, val: intnat) {
    if s.extern_ptr.offset(2) > s.extern_limit {
        grow_extern_output(s, 2);
    }
    *s.extern_ptr.offset(0) = code as c_char;
    *s.extern_ptr.offset(1) = val as c_char;
    s.extern_ptr = s.extern_ptr.offset(2);
}

unsafe fn writecode16(s: &mut caml_extern_state, code: c_int, val: intnat) {
    if s.extern_ptr.offset(3) > s.extern_limit {
        grow_extern_output(s, 3);
    }
    *s.extern_ptr.offset(0) = code as c_char;
    store16(s.extern_ptr.offset(1), val as c_int);
    s.extern_ptr = s.extern_ptr.offset(3);
}

unsafe fn writecode32(s: &mut caml_extern_state, code: c_int, val: intnat) {
    if s.extern_ptr.offset(5) > s.extern_limit {
        grow_extern_output(s, 5);
    }
    *s.extern_ptr.offset(0) = code as c_char;
    store32(s.extern_ptr.offset(1), val);
    s.extern_ptr = s.extern_ptr.offset(5);
}

unsafe fn writecode64(s: &mut caml_extern_state, code: c_int, val: intnat) {
    if s.extern_ptr.offset(9) > s.extern_limit {
        grow_extern_output(s, 9);
    }
    *s.extern_ptr.offset(0) = code as c_char;
    store64(s.extern_ptr.offset(1), val);
    s.extern_ptr = s.extern_ptr.offset(9);
}

/// Marshaling integers
#[inline]
unsafe fn extern_int(s: &mut caml_extern_state, n: intnat) {
    if (0..0x40).contains(&n) {
        write(s, PREFIX_SMALL_INT + n as c_int);
    } else if (-(1 << 7)..(1 << 7)).contains(&n) {
        writecode8(s, CODE_INT8, n);
    } else if (-(1 << 15)..(1 << 15)).contains(&n) {
        writecode16(s, CODE_INT16, n);
    } else if !(-(1 << 30)..(1 << 30)).contains(&n) {
        if s.extern_flags & COMPAT_32 != 0 {
            extern_failwith(
                s,
                b"output_value: integer cannot be read back on 32-bit platform\x00" as *const u8
                    as *const c_char,
            );
        }
        writecode64(s, CODE_INT64, n);
    } else {
        writecode32(s, CODE_INT32, n);
    };
}

/// Marshaling references to previously-marshaled blocks
#[inline]
unsafe fn extern_shared_reference(s: &mut caml_extern_state, d: uintnat) {
    if d < 0x100 {
        writecode8(s, CODE_SHARED8, d as intnat);
    } else if d < 0x10000 {
        writecode16(s, CODE_SHARED16, d as intnat);
    } else if d >= 1 << 32 {
        writecode64(s, CODE_SHARED64, d as intnat);
    } else {
        writecode32(s, CODE_SHARED32, d as intnat);
    };
}

/// Marshaling block headers
#[inline]
unsafe fn extern_header(s: &mut caml_extern_state, sz: mlsize_t, tag: tag_t) {
    if tag < 16 && sz < 8 {
        write(
            s,
            PREFIX_SMALL_BLOCK
                .wrapping_add(tag as c_int)
                .wrapping_add((sz << 4) as c_int),
        );
    } else {
        let hd: header_t = Make_header(sz, tag, NOT_MARKABLE);
        if sz > 0x3FFFFF && s.extern_flags & COMPAT_32 != 0 {
            extern_failwith(
                s,
                b"output_value: array cannot be read back on 32-bit platform\x00" as *const u8
                    as *const c_char,
            );
        }
        if hd < 1 << 32 {
            writecode32(s, CODE_BLOCK32, hd as intnat);
        } else {
            writecode64(s, CODE_BLOCK64, hd as intnat);
        }
    };
}

#[inline]
unsafe fn extern_string(s: &mut caml_extern_state, v: value, len: mlsize_t) {
    if len < 0x20 {
        write(s, PREFIX_SMALL_STRING.wrapping_add(len as c_int));
    } else if len < 0x100 {
        writecode8(s, CODE_STRING8, len as intnat);
    } else {
        if len > 0xFFFFFB && s.extern_flags & COMPAT_32 != 0 {
            extern_failwith(
                s,
                b"output_value: string cannot be read back on 32-bit platform\x00" as *const u8
                    as *const c_char,
            );
        }
        if len < 1 << 32 {
            writecode32(s, CODE_STRING32, len as intnat);
        } else {
            writecode64(s, CODE_STRING64, len as intnat);
        }
    }
    writeblock(s, String_val(v), len as intnat);
}

/// Marshaling FP numbers
#[inline]
unsafe fn extern_double(s: &mut caml_extern_state, v: value) {
    write(s, CODE_DOUBLE_NATIVE);
    writeblock_float8(s, v as *mut c_double, 1 as intnat);
}

/// Marshaling FP arrays
#[inline]
unsafe fn extern_double_array(s: &mut caml_extern_state, v: value, nfloats: mlsize_t) {
    if nfloats < 0x100 {
        writecode8(s, CODE_DOUBLE_ARRAY8_NATIVE, nfloats as intnat);
    } else {
        if nfloats > 0x1FFFFF && s.extern_flags & COMPAT_32 != 0 {
            extern_failwith(
                s,
                b"output_value: float array cannot be read back on 32-bit platform\x00" as *const u8
                    as *const c_char,
            );
        }
        if nfloats < 1 << 32 {
            writecode32(s, CODE_DOUBLE_ARRAY32_NATIVE, nfloats as intnat);
        } else {
            writecode64(s, CODE_DOUBLE_ARRAY64_NATIVE, nfloats as intnat);
        }
    }
    writeblock_float8(s, v as *mut c_double, nfloats as intnat);
}

/// Marshaling custom blocks
#[inline]
unsafe fn extern_custom(
    s: &mut caml_extern_state,
    v: value,
    sz_32: *mut uintnat, // out
    sz_64: *mut uintnat, // out
) {
    let size_header: *mut c_char;
    let ident: *const c_char = (*Custom_ops_val(v)).identifier;
    let serialize: Option<unsafe extern "C" fn(_: value, _: *mut uintnat, _: *mut uintnat) -> ()> =
        (*Custom_ops_val(v)).serialize;
    let fixed_length: *const custom_fixed_length = (*Custom_ops_val(v)).fixed_length;
    let serialize = if let Some(serialize) = serialize {
        serialize
    } else {
        extern_invalid_argument(
            s,
            b"output_value: abstract value (Custom)\x00" as *const u8 as *const c_char,
        );
    };
    if fixed_length.is_null() {
        write(s, CODE_CUSTOM_LEN);
        writeblock(s, ident, strlen(ident).wrapping_add(1) as intnat);
        // Reserve 12 bytes for the lengths (sz_32 and sz_64).
        if s.extern_ptr.offset(12) >= s.extern_limit {
            grow_extern_output(s, 12);
        }
        size_header = s.extern_ptr;
        s.extern_ptr = s.extern_ptr.offset(12);
        serialize(v, sz_32, sz_64);
        // Store length before serialized block
        store32(size_header, *sz_32 as intnat);
        store64(size_header.offset(4), *sz_64 as int64_t);
    } else {
        write(s, CODE_CUSTOM_FIXED);
        writeblock(s, ident, strlen(ident).wrapping_add(1) as intnat);
        serialize(v, sz_32, sz_64);
        if *sz_32 != (*fixed_length).bsize_32 as uintnat
            || *sz_64 != (*fixed_length).bsize_64 as uintnat
        {
            caml_fatal_error(
                b"output_value: incorrect fixed sizes specified by %s\x00" as *const u8
                    as *const c_char,
                ident,
            );
        }
    };
}

// Marshaling code pointers
unsafe fn extern_code_pointer(s: &mut caml_extern_state, codeptr: *mut c_char) {
    let cf: *mut code_fragment;
    let digest: *const c_char;

    cf = caml_find_code_fragment_by_pc(codeptr);
    if !cf.is_null() {
        if s.extern_flags & CLOSURES == 0 {
            extern_invalid_argument(
                s,
                b"output_value: functional value\x00" as *const u8 as *const c_char,
            );
        }
        digest = caml_digest_of_code_fragment(cf) as *const c_char;
        if digest.is_null() {
            extern_invalid_argument(
                s,
                b"output_value: private function\x00" as *const u8 as *const c_char,
            );
        }
        writecode32(
            s,
            CODE_CODEPOINTER,
            codeptr.wrapping_offset_from((*cf).code_start) as c_long,
        );
        writeblock(s, digest, 16);
    } else {
        extern_invalid_argument(
            s,
            b"output_value: abstract value (outside heap)\x00" as *const u8 as *const c_char,
        );
    };
}

/// Marshaling the non-environment part of closures
#[inline]
unsafe fn extern_closure_up_to_env(s: &mut caml_extern_state, v: value) -> mlsize_t {
    let startenv: mlsize_t;
    let mut i: mlsize_t = 0;
    let mut info: value;

    startenv = Start_env_closinfo(Closinfo_val(v));
    loop {
        // The infix header
        if i > 0 {
            let fresh4 = i;
            i = i.wrapping_add(1);
            extern_int(s, Long_val(Field(v, fresh4 as usize)));
        }
        // The default entry point
        let fresh5 = i;
        i = i.wrapping_add(1);
        extern_code_pointer(s, Field(v, fresh5 as usize) as *mut c_char);
        // The closure info.
        let fresh6 = i;
        i = i.wrapping_add(1);
        info = Field(v, fresh6 as usize);
        extern_int(s, Long_val(info));
        // The direct entry point if arity is neither 0 nor 1
        if Arity_closinfo(info) != 0 && Arity_closinfo(info) != 1 {
            let fresh7 = i;
            i = i.wrapping_add(1);
            extern_code_pointer(s, Field(v, fresh7 as usize) as *mut c_char);
        }
        if i >= startenv {
            break;
        }
    }
    startenv
}

/// Marshal the given value in the output buffer
unsafe fn extern_rec(s: &mut caml_extern_state, mut v: value) {
    let mut goto_next_item: bool;

    let mut sp: *mut extern_item;
    let mut h: uintnat = 0;
    let mut pos: uintnat = 0;

    extern_init_position_table(s);
    sp = s.extern_stack;

    loop {
        if Is_long(v) {
            extern_int(s, Long_val(v));
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
                extern_header(s, 0, tag);
            } else {
                // Check if object already seen
                if s.extern_flags & NO_SHARING == 0 {
                    if extern_lookup_position(s, v, &mut pos, &mut h) != 0 {
                        extern_shared_reference(s, s.obj_counter.wrapping_sub(pos));
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
                            extern_string(s, v, len);
                            s.size_32 = s.size_32.wrapping_add(
                                (1 as uintnat).wrapping_add(len.wrapping_add(4).wrapping_div(4)),
                            );
                            s.size_64 = s.size_64.wrapping_add(
                                (1 as uintnat).wrapping_add(len.wrapping_add(8).wrapping_div(8)),
                            );
                            extern_record_location(s, v, h);
                        }
                        Double_tag => {
                            extern_double(s, v);
                            s.size_32 = s.size_32.wrapping_add(1 + 2);
                            s.size_64 = s.size_64.wrapping_add(1 + 1);
                            extern_record_location(s, v, h);
                        }
                        Double_array_tag => {
                            let nfloats: mlsize_t = Wosize_val(v) / Double_wosize;
                            extern_double_array(s, v, nfloats);
                            s.size_32 = (*s)
                                .size_32
                                .wrapping_add((1 as uintnat).wrapping_add(nfloats.wrapping_mul(2)));
                            s.size_64 = (*s)
                                .size_64
                                .wrapping_add((1 as uintnat).wrapping_add(nfloats));
                            extern_record_location(s, v, h);
                        }
                        Abstract_tag => {
                            extern_invalid_argument(
                                s,
                                b"output_value: abstract value (Abstract)\x00" as *const u8
                                    as *const c_char,
                            );
                        }
                        Infix_tag => {
                            writecode32(s, CODE_INFIXPOINTER, Infix_offset_hd(hd) as intnat);
                            v = (v as uintnat).wrapping_sub(Infix_offset_hd(hd)) as value; // PR#5772
                            continue;
                        }
                        Custom_tag => {
                            let mut sz_32: uintnat = 0;
                            let mut sz_64: uintnat = 0;
                            extern_custom(s, v, &mut sz_32, &mut sz_64);
                            s.size_32 = s.size_32.wrapping_add(
                                (2 as uintnat).wrapping_add(sz_32.wrapping_add(3) >> 2),
                            ); // header + ops + data
                            s.size_64 = s.size_64.wrapping_add(
                                (2 as uintnat).wrapping_add(sz_64.wrapping_add(7) >> 3),
                            );
                            extern_record_location(s, v, h);
                        }
                        Closure_tag => {
                            let i: mlsize_t;
                            extern_header(s, sz, tag);
                            s.size_32 = s.size_32.wrapping_add((1 as uintnat).wrapping_add(sz));
                            s.size_64 = s.size_64.wrapping_add((1 as uintnat).wrapping_add(sz));
                            extern_record_location(s, v, h);
                            i = extern_closure_up_to_env(s, v);
                            if i < sz {
                                // Remember that we still have to serialize fields i + 1 ... sz - 1
                                if i < sz.wrapping_sub(1) {
                                    sp = sp.offset(1);
                                    if sp >= s.extern_stack_limit {
                                        sp = extern_resize_stack(s, sp)
                                    }
                                    (*sp).v = Field_ptr_mut(v, i.wrapping_add(1) as usize);
                                    (*sp).count = sz.wrapping_sub(i).wrapping_sub(1)
                                }
                                // Continue serialization with the first environment field
                                v = Field(v, i as usize);
                                continue;
                            }
                        }
                        _ => {
                            extern_header(s, sz, tag);
                            s.size_32 = s.size_32.wrapping_add((1 as uintnat).wrapping_add(sz));
                            s.size_64 = s.size_64.wrapping_add((1 as uintnat).wrapping_add(sz));
                            extern_record_location(s, v, h);
                            // Remember that we still have to serialize fields 1 ... sz - 1
                            if sz > 1 {
                                sp = sp.offset(1);
                                if sp >= s.extern_stack_limit {
                                    sp = extern_resize_stack(s, sp)
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
        if sp == s.extern_stack {
            // We are done.   Cleanup the stack and leave the function
            extern_free_stack(s);
            extern_free_position_table(s);
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

static mut EXTERN_FLAG_VALUES: [c_int; 3] = [NO_SHARING, CLOSURES, COMPAT_32];

unsafe fn extern_value(
    s: &mut caml_extern_state,
    v: value,
    flags: value,
    header: *mut c_char,    // out
    header_len: *mut c_int, // out
) -> intnat {
    let res_len: intnat;
    // Parse flag list
    s.extern_flags = caml_convert_flag_list(flags, EXTERN_FLAG_VALUES.as_ptr());
    // Initializations
    s.obj_counter = 0;
    s.size_32 = 0;
    s.size_64 = 0;
    // Marshal the object
    extern_rec(s, v);
    // Record end of output
    close_extern_output(s);
    // Write the header
    res_len = extern_output_length(s);
    if res_len >= (1 << 32) || s.size_32 >= (1 << 32) || s.size_64 >= (1 << 32) {
        // The object is too big for the small header format.
        // Fail if we are in compat32 mode, or use big header.
        if s.extern_flags & COMPAT_32 != 0 {
            free_extern_output(s);
            caml_failwith(
                b"output_value: object too big to be read back on 32-bit platform\x00" as *const u8
                    as *const c_char,
            );
        }
        store32(header, MAGIC_NUMBER_BIG as intnat);
        store32(header.offset(4), 0);
        store64(header.offset(8), res_len);
        store64(header.offset(16), s.obj_counter as int64_t);
        store64(header.offset(24), s.size_64 as int64_t);
        *header_len = 32;
        return res_len;
    }
    // Use the small header format
    store32(header, MAGIC_NUMBER_SMALL as intnat);
    store32(header.offset(4), res_len);
    store32(header.offset(8), s.obj_counter as intnat);
    store32(header.offset(12), s.size_32 as intnat);
    store32(header.offset(16), s.size_64 as intnat);
    *header_len = 20;
    res_len
}

unsafe fn output_val<W: std::io::Write>(w: &mut W, v: value, flags: value) -> std::io::Result<()> {
    let mut header: [c_char; 32] = [0; 32];
    let mut header_len: c_int = 0;
    let mut blk: *mut output_block;
    let mut nextblk: *mut output_block;
    let s: &mut caml_extern_state = new_extern_state().as_mut().expect("nonnull");
    init_extern_output(s);
    extern_value(s, v, flags, header.as_mut_ptr(), &mut header_len);
    blk = s.extern_output_first;
    free_extern_state(s);
    w.write_all(std::slice::from_raw_parts(
        header.as_mut_ptr() as *mut u8,
        header_len as usize,
    ))?;
    while !blk.is_null() {
        w.write_all(std::slice::from_raw_parts(
            (*blk).data.as_mut_ptr() as *mut u8,
            (*blk).end.wrapping_offset_from((*blk).data.as_mut_ptr()) as usize,
        ))?;
        nextblk = (*blk).next;
        caml_stat_free((*blk).data as caml_stat_block);
        caml_stat_free(blk as caml_stat_block);
        blk = nextblk
    }
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
