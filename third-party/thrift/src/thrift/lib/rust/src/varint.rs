/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

use bytes::Buf;
use bytes::BufMut;

use crate::errors::ProtocolError;
use crate::Result;

pub const MAX_VARINT_U64: usize = 64 / 7 + 1; // max number of bytes for a u64 varint

pub fn write_u64<B: BufMut>(buf: &mut B, v: u64) {
    let mut v = v;
    let acc = &mut [0_u8; MAX_VARINT_U64][..];
    let mut idx = 0;

    #[cfg(debug_assertions)]
    fn get_mut(acc: &mut [u8], idx: usize) -> &mut u8 {
        &mut acc[idx]
    }
    #[cfg(not(debug_assertions))]
    #[inline]
    fn get_mut(acc: &mut [u8], idx: usize) -> &mut u8 {
        unsafe { acc.get_unchecked_mut(idx) }
    }

    while v > 0x7f {
        *get_mut(acc, idx) = 0x80 | ((v as u8) & 0x7f);
        idx += 1;
        v >>= 7;
    }
    *get_mut(acc, idx) = (v as u8) & 0x7f;
    idx += 1;

    buf.put_slice(&acc[..idx]);
}

#[derive(Debug)]
enum Complete {
    Done { used: usize, val: u64 },
    More { used: usize, idx: usize, val: u64 },
}

use self::Complete::*;

pub fn read_u64<B: Buf>(buf: &mut B) -> Result<u64> {
    let mut buf = buf;

    // Operate on byte slices
    fn inner<B: Buf>(off: usize, val: u64, buf: &mut B) -> Result<Complete> {
        let inp = buf.chunk();
        let mut val = val;

        ensure_err!(!inp.is_empty(), ProtocolError::EOF);

        for (idx, v) in inp.iter().enumerate() {
            let shl = idx + off;

            // Make sure its not too long to fit into u64
            ensure_err!(idx < MAX_VARINT_U64, ProtocolError::InvalidValue);

            val += ((v & 0x7f) as u64) << (shl * 7);

            if v & 0x80 == 0 {
                return Ok(Done { used: idx + 1, val });
            }
        }

        // Need more input - entire input has been used
        let used = inp.len();
        Ok(More {
            used,
            idx: off + used,
            val,
        })
    }

    let mut ret = 0; // cumulative result
    let mut off = 0; // number of 7 bit digits in
    loop {
        match inner(off, ret, &mut buf)? {
            Done { used, val } => {
                ret = val;
                buf.advance(used);
                break Ok(ret);
            }
            More { used, idx, val } => {
                ret = val;
                off = idx;
                buf.advance(used);
            }
        }
    }
}

#[inline]
pub fn u64_len(v: u64) -> usize {
    // number of 7 bit units + 1
    if v == 0 {
        1
    } else {
        let max_one = 63 - v.leading_zeros();
        (max_one / 7) as usize + 1
    }
}

#[inline]
pub fn zigzag(v: i64) -> u64 {
    let z = (v << 1) ^ (v >> 63);
    z as u64
}

#[inline]
pub fn unzigzag(v: u64) -> i64 {
    (v >> 1) as i64 ^ -(v as i64 & 1)
}

#[cfg(test)]
mod test {
    use std::i16;
    use std::i32;
    use std::i64;
    use std::i8;
    use std::io::Cursor;
    use std::u64;

    use bufsize::SizeCounter;
    use bytes::BytesMut;
    use quickcheck::quickcheck;

    use super::*;
    use crate::bufext::BufMutExt;

    fn baseline_varint_u64<T: BufMut>(buf: &mut T, v: u64) {
        let mut v = v;

        while v > 0x7f {
            buf.put_u8(0x80 | (v as u8 & 0x7f));
            v >>= 7;
        }
        buf.put_u8(v as u8 & 0x7f);
    }

    #[test]
    fn maxsize() {
        assert_eq!(MAX_VARINT_U64, 10);
    }

    quickcheck! {
        fn check_u64_len(v: u64) -> bool {
            let len = u64_len(v);

            let mut buf = SizeCounter::new();
            baseline_varint_u64(&mut buf, v);

            let ret = buf.finalize();

            len == ret
        }
    }

    quickcheck! {
        fn check_u64(v: u64) -> bool {
            let len = u64_len(v);
            let mut baseline = BytesMut::with_capacity(len);
            let mut cps = BytesMut::with_capacity(len);

            baseline_varint_u64(&mut baseline, v);
            write_u64(&mut cps, v);

            baseline.finalize() == cps.finalize()
        }
    }

    quickcheck! {
        fn zigzag_roundtrip(v: i64) -> bool {
            let z = zigzag(v);
            let u = unzigzag(z);

            v == u
        }
    }

    quickcheck! {
        fn varint_u64_roundtrip(v: u64) -> bool {
            let mut bytes = BytesMut::with_capacity(MAX_VARINT_U64);
            write_u64(&mut bytes, v);
            let bytes = bytes.finalize();

            assert_eq!(bytes.len(), u64_len(v));

            let u = read_u64(&mut Cursor::new(bytes)).expect("decode failed");
            v == u
        }
    }

    quickcheck! {
        fn varint_u64_roundtrip2(a: u64, b: u64) -> bool {
            let mut bytes = BytesMut::with_capacity(MAX_VARINT_U64 * 2);
            write_u64(&mut bytes, a);
            write_u64(&mut bytes, b);
            let bytes = bytes.finalize();

            assert_eq!(bytes.len(), u64_len(a) + u64_len(b));

            let mut cur = Cursor::new(bytes);
            let x = read_u64(&mut cur).expect("decode failed");
            let y = read_u64(&mut cur).expect("decode failed");
            x == a && y == b
        }
    }

    #[test]
    fn simple_1byte() {
        let data = [77_u8];
        const WANT: u64 = 77;
        let mut cur = Cursor::new(&data[..]);

        match read_u64(&mut cur) {
            Ok(WANT) => {}
            Ok(bad) => panic!("Unexpected result {}", bad),
            Err(bad) => panic!("Bad result {:?}", bad),
        }

        assert_eq!(cur.position(), 1);
    }

    #[test]
    fn simple_2byte() {
        let data = [0x80 | 77, 88_u8];
        const WANT: u64 = (88 << 7) + 77;
        let mut cur = Cursor::new(&data[..]);

        match read_u64(&mut cur) {
            Ok(WANT) => {}
            Ok(bad) => panic!("Unexpected result {} wanted {}", bad, WANT),
            Err(bad) => panic!("Bad result {:?}", bad),
        }

        assert_eq!(cur.position(), 2);
    }

    #[test]
    fn simple_3byte() {
        let data = [0x80 | 77, 0x80 | 88, 99_u8];
        const WANT: u64 = (99 << 14) + (88 << 7) + 77;
        let mut cur = Cursor::new(&data[..]);

        match read_u64(&mut cur) {
            Ok(WANT) => {}
            Ok(bad) => panic!("Unexpected result {} wanted {}", bad, WANT),
            Err(bad) => panic!("Bad result {:?}", bad),
        }

        assert_eq!(cur.position(), 3);
    }

    #[test]
    fn simple_3byte_leftover() {
        let data = [0x80 | 77, 0x80 | 88, 99_u8, 44_u8];
        const WANT: u64 = (99 << 14) + (88 << 7) + 77;
        let mut cur = Cursor::new(&data[..]);

        match read_u64(&mut cur) {
            Ok(WANT) => {}
            Ok(bad) => panic!("Unexpected result {} wanted {}", bad, WANT),
            Err(bad) => panic!("Bad result {:?}", bad),
        }

        assert_eq!(cur.position(), 3);
    }

    #[test]
    fn simple_max() {
        let data = [0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01];
        const WANT: u64 = u64::MAX;
        let mut cur = Cursor::new(&data[..]);

        match read_u64(&mut cur) {
            Ok(WANT) => {}
            Ok(bad) => panic!("Unexpected result {} wanted {}", bad, WANT),
            Err(bad) => panic!("Bad result {:?}", bad),
        }

        assert_eq!(cur.position(), 10);
    }

    #[test]
    fn simple_zero() {
        let data = [0x00];
        const WANT: u64 = 0;
        let mut cur = Cursor::new(&data[..]);

        match read_u64(&mut cur) {
            Ok(WANT) => {}
            Ok(bad) => panic!("Unexpected result {} wanted {}", bad, WANT),
            Err(bad) => panic!("Bad result {:?}", bad),
        }

        assert_eq!(cur.position(), 1);
    }

    #[test]
    fn multi_small() {
        let data = [0x80 | 77, 88_u8, 0x80 | 77, 0x80 | 88, 99_u8];
        const WANT1: u64 = (88 << 7) + 77;
        const WANT2: u64 = (99 << 14) + (88 << 7) + 77;
        let mut cur = Cursor::new(&data[..]);

        match read_u64(&mut cur) {
            Ok(WANT1) => {}
            Ok(bad) => panic!("Unexpected result {} wanted {}", bad, WANT1),
            Err(bad) => panic!("Bad result {:?}", bad),
        }
        assert_eq!(cur.position(), 2);

        match read_u64(&mut cur) {
            Ok(WANT2) => {}
            Ok(bad) => panic!("Unexpected result {} wanted {}", bad, WANT2),
            Err(bad) => panic!("Bad result {:?}", bad),
        }

        assert_eq!(cur.position(), 5);
    }

    #[test]
    fn multi_chained() {
        let data1 = [0x80 | 77];
        let data2 = [88_u8, 0x80 | 77];
        let data3 = [0x80 | 88, 99_u8];

        let mut data = Cursor::new(data1)
            .chain(Cursor::new(data2))
            .chain(Cursor::new(data3));

        const WANT1: u64 = (88 << 7) + 77;
        const WANT2: u64 = (99 << 14) + (88 << 7) + 77;

        match read_u64(&mut data) {
            Ok(WANT1) => {}
            Ok(bad) => panic!("Unexpected result {} wanted {}", bad, WANT1),
            Err(bad) => panic!("Bad result {:?}", bad),
        }

        match read_u64(&mut data) {
            Ok(WANT2) => {}
            Ok(bad) => panic!("Unexpected result {} wanted {}", bad, WANT2),
            Err(bad) => panic!("Bad result {:?}", bad),
        }
    }

    #[test]
    fn bad_toolong() {
        let data = [0x80_u8; 11];
        let mut cur = Cursor::new(&data[..]);

        match read_u64(&mut cur) {
            Ok(bad) => panic!("Unexpected result {}", bad),
            Err(err) => match err.downcast_ref::<ProtocolError>() {
                Some(ProtocolError::InvalidValue) => {}
                _ => panic!("Bad result {:?}", err),
            },
        }

        assert_eq!(cur.position(), 0);
    }

    #[test]
    fn bad_tooshort() {
        let data = [0x80_u8];
        let mut cur = Cursor::new(&data[..]);

        match read_u64(&mut cur) {
            Ok(bad) => panic!("Unexpected result {}", bad),
            Err(err) => match err.downcast_ref::<ProtocolError>() {
                Some(_) => {}
                _ => panic!("Bad result {}", err),
            },
        }

        assert_eq!(cur.position(), 1);
    }

    #[test]
    #[ignore] // doesn't fail as expected - returns u64::MAX
    fn bad_toobig() {
        let data = [0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f];
        let mut cur = Cursor::new(&data[..]);

        match read_u64(&mut cur) {
            Ok(bad) => panic!("Unexpected result {}", bad),
            Err(err) => match err.downcast_ref::<ProtocolError>() {
                Some(ProtocolError::InvalidValue) => {}
                _ => panic!("Bad result {:?}", err),
            },
        }

        assert_eq!(cur.position(), 0);
    }

    static INT64_VALUES: &[i64] = &[
        459i64,
        0,
        1,
        -1,
        i8::MIN as i64,
        i8::MIN as i64 + 1,
        i8::MAX as i64,
        i16::MIN as i64,
        i16::MIN as i64 + 1,
        i16::MAX as i64,
        i32::MIN as i64,
        i32::MIN as i64 + 1,
        i32::MAX as i64,
        i64::MIN,
        i64::MIN + 1,
        i64::MAX,
        -2147483535,
        34359738481,
        -35184372088719,
    ];

    #[test]
    fn roundtrip_zigzag() {
        for v in INT64_VALUES.iter() {
            let u = zigzag(*v);
            assert_eq!(*v, unzigzag(u));
        }
    }

    #[test]
    fn roundtrip_list() {
        let mut buf = BytesMut::with_capacity(MAX_VARINT_U64 * INT64_VALUES.len());

        for v in INT64_VALUES.iter() {
            write_u64(&mut buf, zigzag(*v));
        }

        let bytes = buf.finalize();
        let mut cur = Cursor::new(bytes);

        for v in INT64_VALUES.iter() {
            let u = read_u64(&mut cur).map(unzigzag).expect("read failed");

            assert_eq!(*v, u, "mismatch want {} got {}", v, u);
        }
    }
}
