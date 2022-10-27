// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// This file ('deser.rs') was based off c2rust generated code of 'intern.c' at
// revision `15553b77175270d987058b386d737ccb939e8d5a` (i.e. the 4.14.0 tag).

use ocamlrep::Value;

use crate::intext::*;

#[derive(Copy, Clone)]
#[repr(C)]
pub struct MarshalHeader {
    pub magic: u32,
    pub header_len: i32,
    pub data_len: usize,
    pub num_objects: usize,
    pub whsize: usize,
}

struct InternItem<'a> {
    pub dest: *mut Value<'a>,
    pub arg: usize,
    pub op: InternItemStackOp,
}

enum InternItemStackOp {
    ReadItems = 0,
}

struct State<'s, 'a, A> {
    /// Slice holding input data.
    intern_src: &'s [u8],

    /// The allocator of OCaml objects, e.g. `ocamlrep::Arena` or
    /// `ocamlrep_ocamlpool::Pool`.
    alloc: &'a A,

    /// Count how many objects seen so far.
    obj_counter: usize,

    /// Objects already seen.
    intern_obj_table: Vec<Value<'a>>,

    /// The "recursion stack" used in `intern_rec`.
    stack: Vec<InternItem<'a>>,
}

impl<'s, 'a, A: ocamlrep::Allocator> State<'s, 'a, A> {
    const INTERN_STACK_INIT_SIZE: usize = 256;

    fn new(alloc: &'a A, intern_src: &'s [u8]) -> Self {
        Self {
            intern_src,
            alloc,
            obj_counter: 0,
            intern_obj_table: Vec::new(),
            stack: Vec::with_capacity(Self::INTERN_STACK_INIT_SIZE),
        }
    }

    #[inline]
    fn read8u(&mut self) -> u8 {
        let size = std::mem::size_of::<u8>();
        let res = u8::from_be_bytes(self.intern_src[..size].try_into().unwrap());
        self.intern_src = &self.intern_src[size..];
        res
    }

    #[inline]
    fn read8s(&mut self) -> i8 {
        let size = std::mem::size_of::<i8>();
        let res = i8::from_be_bytes(self.intern_src[..size].try_into().unwrap());
        self.intern_src = &self.intern_src[size..];
        res
    }

    #[inline]
    fn read16u(&mut self) -> u16 {
        let size = std::mem::size_of::<u16>();
        let res = u16::from_be_bytes(self.intern_src[..size].try_into().unwrap());
        self.intern_src = &self.intern_src[size..];
        res
    }

    #[inline]
    fn read16s(&mut self) -> i16 {
        let size = std::mem::size_of::<i16>();
        let res = i16::from_be_bytes(self.intern_src[..size].try_into().unwrap());
        self.intern_src = &self.intern_src[size..];
        res
    }

    #[inline]
    fn read32u(&mut self) -> u32 {
        let size = std::mem::size_of::<u32>();
        let res = u32::from_be_bytes(self.intern_src[..size].try_into().unwrap());
        self.intern_src = &self.intern_src[size..];
        res
    }

    #[inline]
    fn read32s(&mut self) -> i32 {
        let size = std::mem::size_of::<i32>();
        let res = i32::from_be_bytes(self.intern_src[..size].try_into().unwrap());
        self.intern_src = &self.intern_src[size..];
        res
    }

    #[inline]
    fn read64u(&mut self) -> u64 {
        let size = std::mem::size_of::<u64>();
        let res = u64::from_be_bytes(self.intern_src[..size].try_into().unwrap());
        self.intern_src = &self.intern_src[8..];
        res
    }

    fn readfloat(&mut self, dst: &mut f64, code: u8) {
        if std::mem::size_of::<f64>() != 8 {
            panic!("input_value: non-standard floats");
        }
        let src: [u8; 8] = self.intern_src[..8].try_into().unwrap();
        self.intern_src = &self.intern_src[8..];
        *dst = match code {
            CODE_DOUBLE_BIG => f64::from_be_bytes(src),
            CODE_DOUBLE_LITTLE => f64::from_le_bytes(src),
            _ => unreachable!(),
        }
    }

    fn readfloats(&mut self, dest: &mut [f64], code: u8) {
        if std::mem::size_of::<f64>() != 8 {
            panic!("input_value: non-standard floats");
        }
        let count = dest.len() * 8; // number of bytes
        let bytes = &self.intern_src[..count];
        self.intern_src = &self.intern_src[count..];
        for (i, bytes) in bytes.chunks_exact(8).enumerate() {
            let src: [u8; 8] = bytes.try_into().unwrap();
            dest[i] = match code {
                CODE_DOUBLE_ARRAY8_BIG | CODE_DOUBLE_ARRAY32_BIG => f64::from_be_bytes(src),
                CODE_DOUBLE_ARRAY8_LITTLE | CODE_DOUBLE_ARRAY32_LITTLE => f64::from_le_bytes(src),
                _ => unreachable!(),
            };
        }
    }

    unsafe fn intern_rec(&mut self, mut dest: *mut Value<'a>) {
        const READ_BLOCK_LABEL: u64 = 16649699497103515194;
        const READ_STRING_LABEL: u64 = 11970676656440271524;
        const READ_SHARED_LABEL: u64 = 8656139126282042408;
        const READ_DOUBLE_ARRAY_LABEL: u64 = 8966088013221564425;
        const NOTHING_TO_DO_LABEL: u64 = 8288085890650723895;

        let mut current_block: u64;
        let mut header: usize;
        let mut code: u8;

        let mut tag: u8 = 0;
        let mut size: usize = 0;
        let mut len: usize = 0;
        let mut v: Value<'a> = Value::from_bits(0);
        let mut ofs: usize = 0;

        // Initially let's try to read the first object from the stream
        self.stack.push(InternItem {
            op: InternItemStackOp::ReadItems,
            dest,
            arg: 1,
        });

        // The un-marshaler loop, the recursion is unrolled
        while let Some(top) = self.stack.last_mut() {
            // Interpret next item on the stack
            dest = top.dest;

            match top.op {
                InternItemStackOp::ReadItems => {
                    // Pop item
                    top.dest = top.dest.offset(1);
                    top.arg -= 1;
                    if top.arg == 0 {
                        self.stack.pop();
                    }
                    // Read a value and set v to this value
                    code = self.read8u() as u8;
                    if code >= PREFIX_SMALL_INT {
                        if code >= PREFIX_SMALL_BLOCK {
                            // Small block
                            tag = code & 0xf;
                            size = (code >> 4 & 0x7) as usize;
                            current_block = READ_BLOCK_LABEL;
                        } else {
                            // Small integer
                            v = Value::int((code & 0x3F) as isize);
                            current_block = NOTHING_TO_DO_LABEL;
                        }
                    } else {
                        if code >= PREFIX_SMALL_STRING {
                            // Small string
                            len = (code & 0x1f) as usize;
                            current_block = READ_STRING_LABEL;
                        } else {
                            match code {
                                CODE_INT8 => {
                                    v = Value::from_bits(
                                        (((self.read8s() as usize) << 1) as usize + 1) as usize,
                                    );
                                    current_block = NOTHING_TO_DO_LABEL;
                                }
                                CODE_INT16 => {
                                    v = Value::from_bits(
                                        (((self.read16s() as usize) << 1) as usize + 1) as usize,
                                    );
                                    current_block = NOTHING_TO_DO_LABEL;
                                }
                                CODE_INT32 => {
                                    v = Value::from_bits(
                                        (((self.read32s() as usize) << 1) as usize + 1) as usize,
                                    );
                                    current_block = NOTHING_TO_DO_LABEL;
                                }
                                CODE_INT64 => {
                                    v = Value::from_bits(
                                        (((self.read64u() as usize) << 1) as usize + 1) as usize,
                                    );
                                    current_block = NOTHING_TO_DO_LABEL;
                                }
                                CODE_SHARED8 => {
                                    ofs = self.read8u() as usize;
                                    current_block = READ_SHARED_LABEL;
                                }
                                CODE_SHARED16 => {
                                    ofs = self.read16u() as usize;
                                    current_block = READ_SHARED_LABEL;
                                }
                                CODE_SHARED32 => {
                                    ofs = self.read32u() as usize;
                                    current_block = READ_SHARED_LABEL;
                                }
                                CODE_SHARED64 => {
                                    ofs = self.read64u() as usize;
                                    current_block = READ_SHARED_LABEL;
                                }
                                CODE_BLOCK32 => {
                                    header = self.read32u() as usize;
                                    tag = (header & 0xff) as u8;
                                    size = header as usize >> 10;
                                    current_block = READ_BLOCK_LABEL;
                                }
                                CODE_BLOCK64 => {
                                    header = self.read64u() as usize;
                                    tag = (header & 0xff) as u8;
                                    size = header as usize >> 10;
                                    current_block = READ_BLOCK_LABEL;
                                }
                                CODE_STRING8 => {
                                    len = self.read8u() as usize;
                                    current_block = READ_STRING_LABEL;
                                }
                                CODE_STRING32 => {
                                    len = self.read32u() as usize;
                                    current_block = READ_STRING_LABEL;
                                }
                                CODE_STRING64 => {
                                    len = self.read64u() as usize;
                                    current_block = READ_STRING_LABEL;
                                }
                                CODE_DOUBLE_LITTLE | CODE_DOUBLE_BIG => {
                                    let mut builder = self.alloc.block_with_size_and_tag(
                                        ocamlrep::DOUBLE_WOSIZE,
                                        ocamlrep::DOUBLE_TAG,
                                    );
                                    self.readfloat(
                                        &mut *(self.alloc.block_ptr_mut(&mut builder) as *mut f64),
                                        code,
                                    );
                                    v = Value::from_bits(builder.build().to_bits());
                                    self.obj_counter += 1;
                                    self.intern_obj_table.push(v);
                                    current_block = NOTHING_TO_DO_LABEL;
                                }
                                CODE_DOUBLE_ARRAY8_LITTLE | CODE_DOUBLE_ARRAY8_BIG => {
                                    len = self.read8u() as usize;
                                    current_block = READ_DOUBLE_ARRAY_LABEL;
                                }
                                CODE_DOUBLE_ARRAY32_LITTLE | CODE_DOUBLE_ARRAY32_BIG => {
                                    len = self.read32u() as usize;
                                    current_block = READ_DOUBLE_ARRAY_LABEL;
                                }
                                CODE_DOUBLE_ARRAY64_LITTLE | CODE_DOUBLE_ARRAY64_BIG => {
                                    len = self.read64u() as usize;
                                    current_block = READ_DOUBLE_ARRAY_LABEL;
                                }
                                CODE_CODEPOINTER | CODE_INFIXPOINTER | CODE_CUSTOM
                                | CODE_CUSTOM_LEN | CODE_CUSTOM_FIXED => {
                                    unimplemented!()
                                }
                                _ => {
                                    panic!("input_value: ill-formed message");
                                }
                            }
                            match current_block {
                                NOTHING_TO_DO_LABEL | READ_BLOCK_LABEL | READ_STRING_LABEL => {}
                                _ => {
                                    match current_block {
                                        READ_SHARED_LABEL => {
                                            v = self.intern_obj_table[(self.obj_counter - ofs) as usize];
                                        }
                                        _ /* READ_DOUBLE_ARRAY_LABEL */ => {
                                            size = len * ocamlrep::DOUBLE_WOSIZE;
                                            let mut builder = self.alloc.block_with_size_and_tag(
                                                size as usize,
                                                ocamlrep::DOUBLE_ARRAY_TAG,
                                            );
                                            self.readfloats(
                                                std::slice::from_raw_parts_mut(self.alloc.block_ptr_mut(&mut builder) as *mut f64, len),
                                                code,
                                            );
                                            v = Value::from_bits(builder.build().to_bits());
                                            self.obj_counter += 1;
                                            self.intern_obj_table.push(v);
                                        }
                                    }
                                    current_block = NOTHING_TO_DO_LABEL;
                                }
                            }
                        }
                        match current_block {
                            NOTHING_TO_DO_LABEL| READ_BLOCK_LABEL => {}
                            _ /* READ_STRING_LABEL */ => {
                                size = (len + std::mem::size_of::<Value<'_>>()) / std::mem::size_of::<Value<'_>>();
                                v = Value::from_bits(ocamlrep::bytes_to_ocamlrep(&self.intern_src[..len], self.alloc).to_bits());
                                self.intern_src = &self.intern_src[len..];
                                self.obj_counter += 1;
                                self.intern_obj_table.push(v);
                                current_block = NOTHING_TO_DO_LABEL;
                            }
                        }
                    }
                    match current_block {
                        READ_BLOCK_LABEL => {
                            if size == 0 {
                                panic!("input_value: atoms are not supported");
                            } else {
                                let mut builder =
                                    self.alloc.block_with_size_and_tag(size as usize, tag);
                                if tag == ocamlrep::OBJECT_TAG {
                                    panic!("input_value: objects not supported");
                                } else {
                                    // If it's not an object then read the
                                    // contents of the block
                                    self.stack.push(InternItem {
                                        op: InternItemStackOp::ReadItems,
                                        dest: self.alloc.block_ptr_mut(&mut builder)
                                            as *mut Value<'a>,
                                        arg: size,
                                    });
                                }
                                v = Value::from_bits(builder.build().to_bits());
                                self.obj_counter += 1;
                                self.intern_obj_table.push(v);
                            }
                        }
                        _ => {}
                    }
                    *dest = v
                }
            }
        }
    }

    unsafe fn parse_header(&mut self, mut h: *mut MarshalHeader) {
        (*h).magic = self.read32u();
        match (*h).magic {
            MAGIC_NUMBER_SMALL => {
                (*h).header_len = 20;
                (*h).data_len = self.read32u() as usize;
                (*h).num_objects = self.read32u() as usize;
                self.read32u();
                (*h).whsize = self.read32u() as usize
            }
            MAGIC_NUMBER_BIG => {
                (*h).header_len = 32;
                self.read32u();
                (*h).data_len = self.read64u() as usize;
                (*h).num_objects = self.read64u() as usize;
                (*h).whsize = self.read64u() as usize;
            }
            _ => panic!("input_value: bad object"),
        };
    }

    unsafe fn input_val_from_string(&mut self, str: &'s [u8]) -> usize {
        let mut obj = Value::int(0).to_bits();
        let mut h = MarshalHeader {
            magic: 0,
            header_len: 0,
            data_len: 0,
            num_objects: 0,
            whsize: 0,
        };
        self.parse_header(&mut h);
        if h.header_len as usize + h.data_len as usize > str.len() {
            panic!("input_value: bad length");
        }
        self.intern_src = &str[(h.header_len as usize)..];
        self.intern_rec(&mut obj as *mut usize as *mut Value<'a>);

        obj
    }
}

pub unsafe fn input_value<'s, 'a, A: ocamlrep::Allocator>(
    str: &'s [u8],
    alloc: &'a A,
) -> ocamlrep::Value<'a> {
    let mut state = State::new(alloc, str);
    ocamlrep::Value::from_bits(state.input_val_from_string(str))
}
