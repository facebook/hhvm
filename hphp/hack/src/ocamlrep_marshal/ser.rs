// Initially generatd by c2rust of 'extern.c' at revision:
// `f14c8ff3f8a164685bc24184fba84904391e378e`.

use std::io;
use std::io::Read;
use std::io::Seek;
use std::io::Write;
use std::mem::MaybeUninit;

use ocamlrep::FromOcamlRep;
use ocamlrep::Header;
use ocamlrep::Value;

use crate::intext::*;

bitflags::bitflags! {
    /// Flags affecting marshaling
    pub struct ExternFlags: u8 {
        /// Flag to ignore sharing
        const NO_SHARING = 1;
        /// Flag to allow marshaling code pointers. Not permitted in `ocamlrep_marshal`.
        const CLOSURES = 2;
        /// Flag to ensure that output can safely be read back on a 32-bit platform
        const COMPAT_32 = 4;
    }
}

// NB: Must match the definition order in ocaml's marshal.ml
#[derive(ocamlrep_derive::FromOcamlRep)]
enum ExternFlag {
    NoSharing,
    Closures,
    Compat32,
}

impl From<ExternFlag> for ExternFlags {
    fn from(flag: ExternFlag) -> Self {
        match flag {
            ExternFlag::NoSharing => ExternFlags::NO_SHARING,
            ExternFlag::Closures => ExternFlags::CLOSURES,
            ExternFlag::Compat32 => ExternFlags::COMPAT_32,
        }
    }
}

impl FromOcamlRep for ExternFlags {
    fn from_ocamlrep(mut list: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        let mut res = ExternFlags::empty();
        while !list.is_immediate() {
            let block = ocamlrep::from::expect_tuple(list, 2)?;
            let flag: ExternFlag = ocamlrep::from::field(block, 0)?;
            res |= flag.into();
            list = block[1];
        }
        Ok(res)
    }
}

// Stack for pending values to marshal

const EXTERN_STACK_INIT_SIZE: usize = 256;
const EXTERN_STACK_MAX_SIZE: usize = 1024 * 1024 * 100;

#[derive(Copy, Clone)]
#[repr(C)]
struct ExternItem<'a> {
    fields: &'a [Value<'a>],
}

// Hash table to record already-marshaled objects and their positions

#[derive(Copy, Clone)]
#[repr(C)]
struct ObjectPosition<'a> {
    obj: Value<'a>,
    pos: usize,
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

#[repr(C)]
struct PositionTable<'a> {
    shift: u8,
    size: usize,           // size == 1 << (wordsize - shift)
    mask: usize,           // mask == size - 1
    threshold: usize,      // threshold == a fixed fraction of size
    present: Box<[usize]>, // [bitvect_size(size)]
    /// SAFETY: Elements of `entries` are not initialized unless their
    /// corresponding bit is set in `present`.
    entries: Box<[MaybeUninit<ObjectPosition<'a>>]>, // [size]
}

const BITS_WORD: usize = 8 * std::mem::size_of::<usize>();

#[inline]
const fn bitvect_size(n: usize) -> usize {
    (n + BITS_WORD - 1) / BITS_WORD
}

const POS_TABLE_INIT_SIZE_LOG2: usize = 8;
const POS_TABLE_INIT_SIZE: usize = 1 << POS_TABLE_INIT_SIZE_LOG2;

// Multiplicative Fibonacci hashing
// (Knuth, TAOCP vol 3, section 6.4, page 518).
// HASH_FACTOR is (sqrt(5) - 1) / 2 * 2^wordsize.
const HASH_FACTOR: usize = 11400714819323198486;
#[inline]
const fn hash(v: Value<'_>, shift: u8) -> usize {
    v.to_bits().wrapping_mul(HASH_FACTOR) >> shift
}

// When the table becomes 2/3 full, its size is increased.
#[inline]
const fn threshold(sz: usize) -> usize {
    (sz * 2) / 3
}

// Accessing bitvectors

#[inline]
fn bitvect_test(bv: &[usize], i: usize) -> bool {
    bv[i / BITS_WORD] & (1 << (i & (BITS_WORD - 1))) != 0
}

#[inline]
fn bitvect_set(bv: &mut [usize], i: usize) {
    bv[i / BITS_WORD] |= 1 << (i & (BITS_WORD - 1));
}

// Conversion to big-endian

#[inline]
fn store16(dst: &mut impl Write, n: i16) -> io::Result<()> {
    dst.write_all(&n.to_be_bytes())
}

#[inline]
fn store32(dst: &mut impl Write, n: i32) -> io::Result<()> {
    dst.write_all(&n.to_be_bytes())
}

#[inline]
fn store64(dst: &mut impl Write, n: i64) -> io::Result<()> {
    dst.write_all(&n.to_be_bytes())
}

#[repr(C)]
struct State<'a, W: Write> {
    flags: ExternFlags, // logical or of some of the flags

    obj_counter: usize, // Number of objects emitted so far
    size_32: usize,     // Size in words of 32-bit block for struct.
    size_64: usize,     // Size in words of 64-bit block for struct.

    // Stack for pending value to marshal
    stack: Vec<ExternItem<'a>>,

    // Hash table to record already marshalled objects
    pos_table: PositionTable<'a>,

    // The output file or buffer we are writing to
    output: W,
    output_len: usize,
}

impl<'a, W: Write> State<'a, W> {
    fn new(output: W) -> Self {
        Self {
            flags: ExternFlags::empty(),
            obj_counter: 0,
            size_32: 0,
            size_64: 0,

            stack: Vec::with_capacity(EXTERN_STACK_INIT_SIZE),

            pos_table: PositionTable {
                shift: 0,
                size: 0,
                mask: 0,
                threshold: 0,
                present: [].into(),
                entries: [].into(),
            },

            output,
            output_len: 0,
        }
    }

    /// Initialize the position table
    fn init_position_table(&mut self) {
        if self.flags.contains(ExternFlags::NO_SHARING) {
            return;
        }
        self.pos_table.size = POS_TABLE_INIT_SIZE;
        self.pos_table.shift =
            (8 * std::mem::size_of::<Value<'a>>() - POS_TABLE_INIT_SIZE_LOG2) as u8;
        self.pos_table.mask = POS_TABLE_INIT_SIZE - 1;
        self.pos_table.threshold = threshold(POS_TABLE_INIT_SIZE);
        // SAFETY: zero is a valid value for the elements of `present`.
        unsafe {
            self.pos_table.present =
                Box::new_zeroed_slice(bitvect_size(POS_TABLE_INIT_SIZE)).assume_init();
        }
        self.pos_table.entries = Box::new_uninit_slice(POS_TABLE_INIT_SIZE);
    }

    /// Grow the position table
    fn resize_position_table(&mut self) {
        let new_size: usize;
        let new_shift: u8;

        // Grow the table quickly (x 8) up to 10^6 entries,
        // more slowly (x 2) afterwards.
        if self.pos_table.size < 1000000 {
            new_size = self.pos_table.size * 8;
            new_shift = self.pos_table.shift - 3;
        } else {
            new_size = self.pos_table.size * 2;
            new_shift = self.pos_table.shift - 1;
        }
        let old = std::mem::replace(
            &mut self.pos_table,
            PositionTable {
                size: new_size,
                shift: new_shift,
                mask: new_size - 1,
                threshold: threshold(new_size),
                // SAFETY: zero is a valid value for the elements of `present`.
                present: unsafe { Box::new_zeroed_slice(bitvect_size(new_size)).assume_init() },
                entries: Box::new_uninit_slice(new_size),
            },
        );

        // Insert every entry of the old table in the new table
        let mut i = 0;
        while i < old.size {
            if bitvect_test(&old.present, i) {
                // SAFETY: We checked that the bit for `i` is set in
                // `old.present`, so `entries[i]` must be initialized
                let old_entry = unsafe { old.entries[i].assume_init() };
                let mut h = hash(old_entry.obj, self.pos_table.shift);
                while bitvect_test(&self.pos_table.present, h) {
                    h = (h + 1) & self.pos_table.mask
                }
                bitvect_set(&mut self.pos_table.present, h);
                self.pos_table.entries[h] = MaybeUninit::new(old_entry);
            }
            i += 1
        }
    }

    /// Determine whether the given object [obj] is in the hash table.
    /// If so, set `*pos_out` to its position in the output and return true.
    /// If not, set `*h_out` to the hash value appropriate for
    /// `record_location` and return false.
    #[inline]
    fn lookup_position(&self, obj: Value<'a>, pos_out: &mut usize, h_out: &mut usize) -> bool {
        let mut h: usize = hash(obj, self.pos_table.shift);
        loop {
            if !bitvect_test(&self.pos_table.present, h) {
                *h_out = h;
                return false;
            }
            // SAFETY: We checked that the bit for `h` is set in `present`, so
            // `entries[h]` must be initialized
            let entry = unsafe { self.pos_table.entries[h].assume_init_ref() };
            if entry.obj == obj {
                *pos_out = entry.pos;
                return true;
            }
            h = (h + 1) & self.pos_table.mask
        }
    }

    /// Record the output position for the given object [obj].
    ///
    /// The [h] parameter is the index in the hash table where the object
    /// must be inserted.  It was determined during lookup.
    fn record_location(&mut self, obj: Value<'a>, h: usize) {
        if self.flags.contains(ExternFlags::NO_SHARING) {
            return;
        }
        bitvect_set(&mut self.pos_table.present, h);
        self.pos_table.entries[h] = MaybeUninit::new(ObjectPosition {
            obj,
            pos: self.obj_counter,
        });
        self.obj_counter += 1;
        if self.obj_counter >= self.pos_table.threshold {
            self.resize_position_table();
        };
    }

    // Write characters, integers, and blocks in the output buffer

    #[inline]
    fn write(&mut self, c: u8) -> io::Result<()> {
        self.output_len += 1;
        self.output.write_all(&[c])
    }

    fn writeblock(&mut self, data: &[u8]) -> io::Result<()> {
        self.output_len += data.len();
        self.output.write_all(data)
    }

    #[inline]
    fn writeblock_float8(&mut self, data: &[f64]) -> io::Result<()> {
        if ARCH_FLOAT_ENDIANNESS == 0x01234567 || ARCH_FLOAT_ENDIANNESS == 0x76543210 {
            // SAFETY: `data.as_ptr()` will be valid for reads of `data.len() *
            // size_of::<f64>()` bytes
            self.writeblock(unsafe {
                std::slice::from_raw_parts(
                    data.as_ptr() as *const u8,
                    data.len() * std::mem::size_of::<f64>(),
                )
            })
        } else {
            unimplemented!()
        }
    }

    fn writecode8(&mut self, code: u8, val: i8) -> io::Result<()> {
        self.output_len += 2;
        self.output.write_all(&[code, val as u8])
    }

    fn writecode16(&mut self, code: u8, val: i16) -> io::Result<()> {
        self.output_len += 3;
        self.output.write_all(&[code])?;
        store16(&mut self.output, val)
    }

    fn writecode32(&mut self, code: u8, val: i32) -> io::Result<()> {
        self.output_len += 5;
        self.output.write_all(&[code])?;
        store32(&mut self.output, val)
    }

    fn writecode64(&mut self, code: u8, val: i64) -> io::Result<()> {
        self.output_len += 9;
        self.output.write_all(&[code])?;
        store64(&mut self.output, val)
    }

    /// Marshaling integers
    #[inline]
    fn extern_int(&mut self, n: isize) -> io::Result<()> {
        if (0..0x40).contains(&n) {
            self.write(PREFIX_SMALL_INT + n as u8)
        } else if (-(1 << 7)..(1 << 7)).contains(&n) {
            self.writecode8(CODE_INT8, n as i8)
        } else if (-(1 << 15)..(1 << 15)).contains(&n) {
            self.writecode16(CODE_INT16, n as i16)
        } else if !(-(1 << 30)..(1 << 30)).contains(&n) {
            if self.flags.contains(ExternFlags::COMPAT_32) {
                panic!("output_value: integer cannot be read back on 32-bit platform");
            }
            self.writecode64(CODE_INT64, n as i64)
        } else {
            self.writecode32(CODE_INT32, n as i32)
        }
    }

    /// Marshaling references to previously-marshaled blocks
    #[inline]
    fn extern_shared_reference(&mut self, d: usize) -> io::Result<()> {
        if d < 0x100 {
            self.writecode8(CODE_SHARED8, d as i8)
        } else if d < 0x10000 {
            self.writecode16(CODE_SHARED16, d as i16)
        } else if d >= 1 << 32 {
            self.writecode64(CODE_SHARED64, d as i64)
        } else {
            self.writecode32(CODE_SHARED32, d as i32)
        }
    }

    /// Marshaling block headers
    #[inline]
    fn extern_header(&mut self, sz: usize, tag: u8) -> io::Result<()> {
        if tag < 16 && sz < 8 {
            self.write(PREFIX_SMALL_BLOCK + tag + ((sz as u8) << 4))
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
            let hd = Header::with_color(sz, tag, ocamlrep::Color::White).to_bits();

            if sz > 0x3FFFFF && self.flags.contains(ExternFlags::COMPAT_32) {
                panic!("output_value: array cannot be read back on 32-bit platform");
            }
            if hd < 1 << 32 {
                self.writecode32(CODE_BLOCK32, hd as i32)
            } else {
                self.writecode64(CODE_BLOCK64, hd as i64)
            }
        }
    }

    #[inline]
    fn extern_string(&mut self, bytes: &'a [u8]) -> io::Result<()> {
        let len = bytes.len();
        if len < 0x20 {
            self.write(PREFIX_SMALL_STRING + len as u8)?;
        } else if len < 0x100 {
            self.writecode8(CODE_STRING8, len as i8)?;
        } else {
            if len > 0xFFFFFB && self.flags.contains(ExternFlags::COMPAT_32) {
                panic!("output_value: string cannot be read back on 32-bit platform");
            }
            if len < 1 << 32 {
                self.writecode32(CODE_STRING32, len as i32)?;
            } else {
                self.writecode64(CODE_STRING64, len as i64)?;
            }
        }
        self.writeblock(bytes)
    }

    /// Marshaling FP numbers
    #[inline]
    fn extern_double(&mut self, v: f64) -> io::Result<()> {
        self.write(CODE_DOUBLE_NATIVE)?;
        self.writeblock_float8(&[v])
    }

    /// Marshaling FP arrays
    #[inline]
    fn extern_double_array(&mut self, slice: &[f64]) -> io::Result<()> {
        let nfloats = slice.len();
        if nfloats < 0x100 {
            self.writecode8(CODE_DOUBLE_ARRAY8_NATIVE, nfloats as i8)?;
        } else {
            if nfloats > 0x1FFFFF && self.flags.contains(ExternFlags::COMPAT_32) {
                panic!("output_value: float array cannot be read back on 32-bit platform");
            }
            if nfloats < 1 << 32 {
                self.writecode32(CODE_DOUBLE_ARRAY32_NATIVE, nfloats as i32)?;
            } else {
                self.writecode64(CODE_DOUBLE_ARRAY64_NATIVE, nfloats as i64)?;
            }
        }
        self.writeblock_float8(slice)
    }

    /// Marshal the given value in the output buffer
    fn extern_rec(&mut self, mut v: Value<'a>) -> io::Result<()> {
        let mut goto_next_item: bool;

        let mut h: usize = 0;
        let mut pos: usize = 0;

        self.init_position_table();

        loop {
            if v.is_immediate() {
                self.extern_int(v.as_int().unwrap())?;
            } else {
                let b = v.as_block().unwrap();
                let tag = b.tag();
                let sz = b.size();

                if tag == ocamlrep::FORWARD_TAG {
                    let f = b[0];
                    if let Some(f) = f.as_block()
                        && (f.tag() == ocamlrep::FORWARD_TAG
                            || f.tag() == ocamlrep::LAZY_TAG
                            || f.tag() == ocamlrep::FORCING_TAG
                            || f.tag() == ocamlrep::DOUBLE_TAG)
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
                    self.extern_header(0, tag)?;
                } else {
                    // Check if object already seen
                    if !self.flags.contains(ExternFlags::NO_SHARING) {
                        if self.lookup_position(v, &mut pos, &mut h) {
                            self.extern_shared_reference(self.obj_counter - pos)?;
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
                            ocamlrep::STRING_TAG => {
                                let bytes = v.as_byte_string().unwrap();
                                let len: usize = bytes.len();
                                self.extern_string(bytes)?;
                                self.size_32 += 1 + (len + 4) / 4;
                                self.size_64 += 1 + (len + 8) / 8;
                                self.record_location(v, h);
                            }
                            ocamlrep::DOUBLE_TAG => {
                                self.extern_double(v.as_float().unwrap())?;
                                self.size_32 += 1 + 2;
                                self.size_64 += 1 + 1;
                                self.record_location(v, h);
                            }
                            ocamlrep::DOUBLE_ARRAY_TAG => {
                                let slice = v.as_double_array().unwrap();
                                self.extern_double_array(slice)?;
                                let nfloats = slice.len();
                                self.size_32 += 1 + nfloats * 2;
                                self.size_64 += 1 + nfloats;
                                self.record_location(v, h);
                            }
                            ocamlrep::ABSTRACT_TAG => {
                                panic!("output_value: abstract value (Abstract)");
                            }
                            // INFIX_TAG represents an infix header inside a
                            // closure, and can only occur in blocks with tag
                            // CLOSURE_TAG
                            ocamlrep::INFIX_TAG => {
                                panic!("output_value: marshaling of closures not implemented");
                            }
                            ocamlrep::CUSTOM_TAG => {
                                panic!("output_value: marshaling of custom blocks not implemented");
                            }
                            ocamlrep::CLOSURE_TAG => {
                                panic!("output_value: marshaling of closures not implemented");
                            }
                            _ => {
                                self.extern_header(sz, tag)?;
                                self.size_32 += 1 + sz;
                                self.size_64 += 1 + sz;
                                self.record_location(v, h);
                                // Remember that we still have to serialize fields 1 ... sz - 1
                                if sz > 1 {
                                    if self.stack.len() + 1 >= EXTERN_STACK_MAX_SIZE {
                                        panic!("Stack overflow in marshaling value");
                                    }
                                    self.stack.push(ExternItem {
                                        fields: &b.as_values().unwrap()[1..],
                                    });
                                }
                                // Continue serialization with the first field
                                v = v.field(0).unwrap();
                                continue;
                            }
                        }
                    }
                }
            }
            // C goto label `next_item:` here

            // Pop one more item to marshal, if any
            if let Some(item) = self.stack.last_mut() {
                v = item.fields[0];
                item.fields = &item.fields[1..];
                if item.fields.is_empty() {
                    self.stack.pop();
                }
            } else {
                // We are done.
                return Ok(());
            }
        }
        // Never reached as function leaves with return
    }

    fn extern_value(
        &mut self,
        v: Value<'a>,
        flags: ExternFlags,
        mut header: &mut [u8],  // out
        header_len: &mut usize, // out
    ) -> io::Result<usize> {
        // Initializations
        self.flags = flags;
        self.obj_counter = 0;
        self.size_32 = 0;
        self.size_64 = 0;
        // Marshal the object
        self.extern_rec(v)?;
        // Write the header
        let res_len = self.output_len;
        if res_len >= (1 << 32) || self.size_32 >= (1 << 32) || self.size_64 >= (1 << 32) {
            // The object is too big for the small header format.
            // Fail if we are in compat32 mode, or use big header.
            if self.flags.contains(ExternFlags::COMPAT_32) {
                panic!("output_value: object too big to be read back on 32-bit platform");
            }
            store32(&mut header, MAGIC_NUMBER_BIG as i32)?;
            store32(&mut header, 0)?;
            store64(&mut header, res_len as i64)?;
            store64(&mut header, self.obj_counter as i64)?;
            store64(&mut header, self.size_64 as i64)?;
            *header_len = 32;
            Ok(res_len)
        } else {
            // Use the small header format
            store32(&mut header, MAGIC_NUMBER_SMALL as i32)?;
            store32(&mut header, res_len as i32)?;
            store32(&mut header, self.obj_counter as i32)?;
            store32(&mut header, self.size_32 as i32)?;
            store32(&mut header, self.size_64 as i32)?;
            *header_len = 20;
            Ok(res_len)
        }
    }
}

pub fn output_value<W: Read + Write + Seek>(
    w: &mut W,
    v: Value<'_>,
    flags: ExternFlags,
) -> io::Result<()> {
    let mut header = [0; 32];
    let mut header_len = 0;
    // At this point we don't know the size of the header.
    // Guess that it is small, and fix up later if not.
    w.seek(std::io::SeekFrom::Start(0))?;
    w.write_all(&[0; 20])?;
    let mut s = State::new(&mut *w);
    s.extern_value(v, flags, &mut header, &mut header_len)?;
    drop(s);
    w.flush()?;
    if header_len != 20 {
        // Bad guess! Need to shift the output to make room for big header.
        w.seek(std::io::SeekFrom::Start(20))?;
        let mut output = vec![];
        w.read_to_end(&mut output)?;
        w.seek(std::io::SeekFrom::Start(header_len as u64))?;
        w.write_all(&output)?;
    }
    w.seek(std::io::SeekFrom::Start(0))?;
    w.write_all(&header[0..header_len])?;
    w.flush()
}
