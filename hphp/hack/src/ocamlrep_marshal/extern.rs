// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod v0 {

    #![allow(dead_code)] //for now

    use once_cell::sync::OnceCell;

    use crate::intext::SIZE_EXTERN_OUTPUT_BLOCK;

    /// Flags affecting marshaling
    const NO_SHARING: i32 = 1; // Ignore sharing
    const CLOSURES: i32 = 2; // Allow marshaling code pointers
    const COMPAT_32: i32 = 4; // Ensure output can be read back on a 32-bit platform

    // Stack for pending values to marshal

    const EXTERN_STACK_INIT_SIZE: usize = 256;
    const EXTERN_STACK_MAX_SIZE: usize = 1024 * 1024 * 100;

    // [Note: Safety: Not really sync]
    // -------------------------------
    // This type contains (raw) pointers: technically the type is `Sync` (c.f.
    // constraint requirement on `OnceCell`) however, values of this kind can't be
    // safely shared between threads - there is no way to prevent data races.

    #[derive(Copy, Clone, Default)]
    #[repr(C)]
    struct ExternItem {
        // See [Note: Safety: Not really sync].
        v: usize,     // C: `value * v;`
        count: usize, // C: `mlsize_t count;`
    }

    // Hash table to record already-marshaled objects and their positions.

    #[derive(Copy, Clone, Default)]
    #[repr(C)]
    struct ObjectPosition {
        // See [Note: Safety: Not really sync].
        obj: usize, // C: `value obj;`
        pos: usize, // C: `uintnat pos;`
    }

    ///   The hash table uses open addressing, linear probing, and a redundant
    ///   representation:
    ///   - a bitvector `present` records which entries of the table are
    ///     occupied;
    ///   - an array `entries` records (object, position) pairs for the entries
    ///     that are occupied.
    ///   The bitvector is much smaller than the array (1/128th on 64-bit
    ///   platforms, 1/64th on 32-bit platforms), so it has better locality,
    ///   making it faster to determine that an object is not in the table.
    ///   Also, it makes it faster to empty or initialize a table: only the
    ///   `present` bitvector needs to be filled with zeros, the [entries] array
    ///   can be left uninitialized.

    #[derive(Default, Copy, Clone)]
    #[repr(C)]
    struct PositionTable {
        // See [Note: Safety: Not really sync].
        shift: i32,
        size: usize,      // size == 1 << (wordsize - shift)
        mask: usize,      // mask == size - 1
        threshold: usize, // threshold == a fixed fraction of size
        present: usize,   // C: `uintnat * ([bitvect_size(size)])`
        entries: usize,   // C: `struct object_position *` ([size])
    }

    const BITS_WORD: usize = 8 * std::mem::size_of::<usize>();

    // Calculate the number of words necessary for enough bits to have one bit for
    // each one of `n` elements.
    const fn bitvect_size(n: usize) -> usize {
        (n + BITS_WORD - 1) / BITS_WORD
    }

    const POS_TABLE_INIT_SIZE_LOG2: usize = 8;
    const POS_TABLE_INIT_SIZE: usize = 1 << POS_TABLE_INIT_SIZE_LOG2;

    #[derive(Copy, Clone)]
    #[repr(C)]
    struct OutputBlock {
        // See [Note: Safety: Not really sync].
        next: usize, // C: `struct output_block *next`,
        end: usize,  // C: `char *`

        // Originally, this:
        //   char data[SIZE_EXTERN_OUTPUT_BLOCK];
        size: usize,
        data: usize, // treat this as a ptr to a slice of char of len size
    }

    impl Default for OutputBlock {
        fn default() -> Self {
            Self::new(SIZE_EXTERN_OUTPUT_BLOCK)
        }
    }

    impl OutputBlock {
        fn new(size: usize) -> Self {
            Self {
                next: 0usize,
                end: 0usize,
                size,
                data: Box::leak(vec![0 as libc::c_char; size].into_boxed_slice()).as_ptr() as usize,
            }
        }
    }

    #[repr(C)]
    struct CamlExternState {
        // See [Note: Safety: Not really sync].
        extern_flags: i32, // logical or of some of the flags

        obj_counter: usize, // number of objects emitted so far
        size_32: usize,     // size in words of 32-bit block for struct
        size_64: usize,     // size in words of 64-bit block for struct

        // Stack for pending value to marshal
        extern_stack_init: [ExternItem; EXTERN_STACK_INIT_SIZE],
        extern_stack: usize,       // C: `struct extern_item* `
        extern_stack_limit: usize, // C: `struct extern_item*`

        // Hash table to record already marshaled objects
        pos_table_present_init: [usize; bitvect_size(POS_TABLE_INIT_SIZE)],
        pos_table_entries_init: [ObjectPosition; POS_TABLE_INIT_SIZE],
        pos_table: PositionTable,

        // To buffer the output
        extern_userprovided_output: usize, // C: `char *`
        extern_ptr: usize,                 // C: `char *`
        extern_limit: usize,               // C: `char *`

        extern_output_first: usize, // C: `struct output_block *`
        extern_output_block: usize, // C: `struct output_block *`
    }

    impl CamlExternState {
        fn new() -> CamlExternState {
            CamlExternState {
                extern_flags: 0i32,
                obj_counter: 0usize,
                size_32: 0usize,
                size_64: 0usize,
                extern_stack_init: [ExternItem::default(); EXTERN_STACK_INIT_SIZE],
                extern_stack: 0usize,
                extern_stack_limit: 0usize,
                pos_table_present_init: [0usize; bitvect_size(POS_TABLE_INIT_SIZE)],
                pos_table_entries_init: [ObjectPosition::default(); POS_TABLE_INIT_SIZE],
                pos_table: PositionTable::default(),
                extern_userprovided_output: 0usize,
                extern_ptr: 0usize,
                extern_limit: 0usize,
                extern_output_first: 0usize,
                extern_output_block: 0usize,
            }
        }
    }

    unsafe fn get_extern_state() -> &'static mut CamlExternState {
        static mut INSTANCE: OnceCell<Box<CamlExternState>> = OnceCell::new();

        INSTANCE.get_or_init(|| {
            let mut extern_state = Box::new(CamlExternState::new());

            let extern_stack_init = extern_state.extern_stack_init.as_ptr();
            extern_state.extern_stack = extern_stack_init as usize;
            extern_state.extern_stack_limit =
                extern_stack_init.add(EXTERN_STACK_INIT_SIZE) as usize;

            extern_state
        });

        &mut *(INSTANCE.get_mut().unwrap())
    }

    fn extern_free_stack(s: &mut CamlExternState) {
        // Free the extern stack if needed.
        if s.extern_stack != s.extern_stack_init.as_ptr() as usize {
            let len = s.extern_stack_limit - s.extern_stack;
            let begin = s.extern_stack as *mut ExternItem;
            unsafe { Box::from_raw(std::slice::from_raw_parts_mut(begin, len)) };
        }
        // Re-initialize the globals for next time round.
        let extern_stack_init = s.extern_stack_init.as_ptr();
        s.extern_stack = extern_stack_init as usize;
        s.extern_stack_limit = unsafe { extern_stack_init.add(EXTERN_STACK_INIT_SIZE) } as usize;
    }

    fn extern_resize_stack(s: &mut CamlExternState, sp: usize) {
        let len = s.extern_stack_limit - s.extern_stack;
        let newsize = 2 * len;
        let sp_offset = sp - s.extern_stack;
        let begin = s.extern_stack as *mut ExternItem;

        if newsize >= EXTERN_STACK_MAX_SIZE {
            panic!("ocamlrep_marshal.extern: Stack overflow in marshaling value");
        }

        let mut newstack = vec![ExternItem::default(); newsize].into_boxed_slice();

        // Copy items from the old stack to the new stack.
        newstack[..sp_offset]
            .copy_from_slice(unsafe { std::slice::from_raw_parts_mut(begin, sp_offset) });

        // Free the old stack if it is not the initial stack.
        if s.extern_stack != s.extern_stack_init.as_ptr() as usize {
            unsafe { Box::from_raw(std::slice::from_raw_parts_mut(begin, len)) };
        }

        let sp = Box::leak(newstack).as_mut_ptr();
        s.extern_stack = sp as usize;
        s.extern_stack_limit = unsafe { sp.add(newsize) } as usize;
    }

    ///   Multiplicative Fibonacci hashing
    ///   (Knuth, TAOCP vol 3, section 6.4, page 518).
    ///   HASH_FACTOR is (sqrt(5) - 1) / 2 * 2^wordsize.

    // ---
    // c.f. runtime/caml/ml.h.in
    // "Define ARCH_SIXTYFOUR if the processor has a natural word size of 64
    // bits. That is, sizeof(char *) = 8. Otherwise, leave ARCH_SIXTYFOUR
    // undefined. This assumes sizeof(char *) = 4."

    #[cfg(target_pointer_width = "64")]
    const HASH_FACTOR: usize = 11400714819323198486usize;

    #[cfg(target_pointer_width = "32")]
    const HASH_FACTOR: usize = 2654435769usize;

    const fn hash(v: usize, shift: i32) -> usize {
        (v * HASH_FACTOR) >> shift
    }

    // When the table becomes 2/3 full, its size is increased.
    fn threshold(sz: usize) -> usize {
        (sz * 2) / 3
    }

    // Initialize the position table.

    fn extern_init_position_table(s: &mut CamlExternState) {
        if s.extern_flags & NO_SHARING != 0 {
            return;
        }
        s.pos_table.size = POS_TABLE_INIT_SIZE;
        s.pos_table.shift = (BITS_WORD - POS_TABLE_INIT_SIZE_LOG2) as i32;
        s.pos_table.mask = POS_TABLE_INIT_SIZE - 1;
        s.pos_table.threshold = threshold(POS_TABLE_INIT_SIZE);
        s.pos_table.present = s.pos_table_present_init.as_ptr() as usize;
        s.pos_table.entries = s.pos_table_entries_init.as_ptr() as usize;
        s.pos_table_present_init.fill(0usize);
    }

    // Free the position table.

    fn extern_free_position_table(s: &mut CamlExternState) {
        if s.extern_flags & NO_SHARING != 0 {
            return;
        }
        if s.pos_table.present != s.pos_table_present_init.as_ptr() as usize {
            let begin = s.pos_table.present as *mut usize;
            unsafe {
                Box::from_raw(std::slice::from_raw_parts_mut(
                    begin,
                    bitvect_size(s.pos_table.size),
                ))
            };

            let begin = s.pos_table.entries as *mut ObjectPosition;
            unsafe { Box::from_raw(std::slice::from_raw_parts_mut(begin, s.pos_table.size)) };

            // Protect against repeated calls to `extern_free_position_table`.
            s.pos_table.present = s.pos_table_present_init.as_ptr() as usize;
            s.pos_table.entries = s.pos_table_entries_init.as_ptr() as usize;
        }
    }

    // Accessing bitvectors

    #[inline(always)]
    fn bitvect_test(bv: usize, i: usize) -> usize {
        let p = bv as *const usize;
        (unsafe { *p.add(i / BITS_WORD) }) & (1usize << (i & (BITS_WORD - 1)))
    }

    #[inline(always)]
    fn bitvect_set(bv: usize, i: usize) {
        let p = bv as *mut usize;
        unsafe {
            std::ptr::write(
                p.add(i / BITS_WORD),
                *(p.add(i / BITS_WORD)) | (1usize << (i & (BITS_WORD - 1))),
            )
        }
    }

    // Grow the position table

    fn extern_resize_position_table(s: &mut CamlExternState) {
        let old: PositionTable = s.pos_table;

        // Grow the table quickly (x 8) up to 10^6 entries, more slowly (x 2)
        // afterwards.
        let new_shift: i32;
        let new_size: usize;
        if old.size < 1000000 {
            new_size = 8 * old.size;
            new_shift = old.shift - 3;
        } else {
            new_size = 2 * old.size;
            new_shift = old.shift - 1;
        }
        if new_size == 0 {
            panic!("ocamlrep_marshal.extern: extern_resize_position_table: new_size == 0");
        }

        let new_entries: usize =
            Box::leak(vec![ObjectPosition::default(); new_size].into_boxed_slice()).as_ptr()
                as usize;
        let new_present: usize =
            Box::leak(vec![0usize; bitvect_size(new_size)].into_boxed_slice()).as_ptr() as usize;

        s.pos_table.size = new_size;
        s.pos_table.shift = new_shift;
        s.pos_table.mask = new_size - 1;
        s.pos_table.threshold = threshold(new_size);
        s.pos_table.present = new_present;
        s.pos_table.entries = new_entries;

        // Insert every entry of the old table in the new table.
        let old_entries_base = old.entries as *const ObjectPosition;
        let new_entries_base = new_entries as *mut ObjectPosition;
        for i in 0..old.size {
            if bitvect_test(old.present, i) == 0 {
                continue;
            }
            let obj = unsafe { *old_entries_base.add(i) }.obj;
            let mut h = hash(obj, s.pos_table.shift);
            while bitvect_test(new_present, h) != 0 {
                h = (h + 1) & s.pos_table.mask;
            }
            bitvect_set(new_present, h);
            unsafe { *new_entries_base.add(h) = *old_entries_base.add(i) };
        }

        // Free the old tables if they are not the initial ones
        if old.present != s.pos_table_present_init.as_ptr() as usize {
            let begin = old.present as *mut usize;
            unsafe {
                Box::from_raw(std::slice::from_raw_parts_mut(
                    begin,
                    bitvect_size(old.size),
                ))
            };
            let begin = old.entries as *mut ObjectPosition;
            unsafe { Box::from_raw(std::slice::from_raw_parts_mut(begin, old.size)) };
        }
    }

    // Determine whether the given object `obj` is in the hash table. If so, set
    // `*pos_out` to its position in the output and return 1. If not, set
    // `*h_out` to the hash value appropriate for `extern_record_location` and
    // `return 0`.

    fn extern_lookup_position(
        s: &mut CamlExternState,
        obj: usize,     // C: `value`
        pos_out: usize, // C: `uintnat *`
        h_out: usize,   // C: `uintnat *`
    ) -> i32 {
        let mut h = hash(obj, s.pos_table.shift);
        loop {
            if bitvect_test(s.pos_table.present, h) == 0 {
                let h_out = h_out as *mut usize;
                unsafe { std::ptr::write(h_out, h) };
                return 0;
            }
            let pos_table_entries = s.pos_table.entries as *const ObjectPosition;
            if unsafe { *pos_table_entries.add(h) }.obj == obj {
                let pos_out = pos_out as *mut usize;
                unsafe { std::ptr::write(pos_out, (*pos_table_entries.add(h)).pos) };
                return 1;
            }
            h = (h + 1) & s.pos_table.mask;
        }
    }

    // Record the output position for the given object `obj`. The `h` parameter
    // is the index in the hash table where the object must be inserted. It was
    // determined during lookup.

    fn extern_record_location(s: &mut CamlExternState, obj: usize, h: usize) {
        if s.extern_flags & NO_SHARING != 0 {
            return;
        }
        bitvect_set(s.pos_table.present, h);
        let pos_table_entries = s.pos_table.entries as *const ObjectPosition;
        unsafe { *pos_table_entries.add(h) }.obj = obj;
        unsafe { *pos_table_entries.add(h) }.pos = s.obj_counter;
        s.obj_counter += 1;
        if s.obj_counter >= s.pos_table.threshold {
            extern_resize_position_table(s);
        }
    }

    // To buffer the output

    fn init_extern_output(s: &mut CamlExternState) {
        s.extern_userprovided_output = 0usize;
        s.extern_output_first = Box::into_raw(Box::new(OutputBlock::default())) as usize;
        s.extern_output_block = s.extern_output_first;
        let s_extern_output_block = s.extern_output_block as *mut OutputBlock;
        unsafe { *s_extern_output_block }.next = 0usize;
        s.extern_ptr = unsafe { *s_extern_output_block }.data;
        s.extern_limit = unsafe { *s_extern_output_block }.data + SIZE_EXTERN_OUTPUT_BLOCK;
    }

    fn close_extern_output(s: &mut CamlExternState) {
        let s_extern_output_block;
        if s.extern_userprovided_output == 0 {
            s_extern_output_block = s.extern_output_block as *mut OutputBlock;
            unsafe { *s_extern_output_block }.end = s.extern_ptr;
        }
    }

    fn free_extern_output(s: &mut CamlExternState) {
        let mut blk: *mut OutputBlock;
        let mut nextblk: *mut OutputBlock;

        if s.extern_userprovided_output == 0 {
            blk = s.extern_output_first as *mut OutputBlock;
            while !blk.is_null() {
                nextblk = unsafe { (*blk).next as *mut OutputBlock };
                unsafe {
                    Box::from_raw(std::slice::from_raw_parts_mut(
                        (*blk).data as *mut libc::c_char,
                        (*blk).size,
                    ))
                };
                unsafe { Box::from_raw(blk) };
                blk = nextblk;
            }
            s.extern_output_first = 0usize;
        }
        extern_free_stack(s);
        extern_free_position_table(s);
    }

    fn grow_extern_output(s: &mut CamlExternState, required: isize) {
        let mut s_extern_output_block: *mut OutputBlock;

        if s.extern_userprovided_output != 0 {
            panic!("ocamlrep_marshal.extern: Marshal.to_buffer: buffer overflow");
        }

        s_extern_output_block = s.extern_output_block as *mut OutputBlock;
        unsafe { *s_extern_output_block }.end = s.extern_ptr;
        let extra = if required <= (SIZE_EXTERN_OUTPUT_BLOCK / 2) as isize {
            0
        } else {
            required as usize
        };
        let blk =
            Box::into_raw(Box::new(OutputBlock::new(SIZE_EXTERN_OUTPUT_BLOCK + extra))) as usize;
        unsafe { *s_extern_output_block }.next = blk;
        s.extern_output_block = blk;
        s_extern_output_block = s.extern_output_block as *mut OutputBlock;
        unsafe { *s_extern_output_block }.next = 0usize;
        s.extern_ptr = unsafe { *s_extern_output_block }.data;
        s.extern_limit = unsafe { *s_extern_output_block }.data + SIZE_EXTERN_OUTPUT_BLOCK + extra;
    }

    fn extern_output_length(s: &mut CamlExternState) -> isize {
        let mut len: isize = 0;
        let mut blk: *const OutputBlock;
        if s.extern_userprovided_output != 0 {
            return (s.extern_ptr - s.extern_userprovided_output) as isize;
        } else {
            blk = s.extern_output_first as *const OutputBlock;
            while !blk.is_null() {
                len += unsafe { (*blk).end - (*blk).data } as isize;
                blk = unsafe { *blk }.next as *const OutputBlock;
            }
        }
        len
    }

    // Conversion to big-endian.

    fn store16(dst: *mut libc::c_char, n: libc::c_int) {
        unsafe {
            let dst: &mut [libc::c_char] = std::slice::from_raw_parts_mut(dst, 2);
            dst[0] = (n >> 8) as libc::c_char;
            dst[1] = n as libc::c_char;
        }
    }

    fn store32(dst: *mut libc::c_char, n: isize) {
        unsafe {
            let dst: &mut [libc::c_char] = std::slice::from_raw_parts_mut(dst, 4);
            dst[0] = (n >> 24) as libc::c_char;
            dst[1] = (n >> 18) as libc::c_char;
            dst[2] = (n >> 8) as libc::c_char;
            dst[3] = n as libc::c_char;
        }
    }

    fn store64(dst: *mut libc::c_char, n: i64) {
        unsafe {
            let dst: &mut [libc::c_char] = std::slice::from_raw_parts_mut(dst, 8);
            dst[0] = (n >> 56) as libc::c_char;
            dst[1] = (n >> 48) as libc::c_char;
            dst[2] = (n >> 40) as libc::c_char;
            dst[3] = (n >> 32) as libc::c_char;
            dst[4] = (n >> 24) as libc::c_char;
            dst[5] = (n >> 16) as libc::c_char;
            dst[6] = (n >> 8) as libc::c_char;
            dst[7] = n as libc::c_char;
        }
    }
} //mod v0
