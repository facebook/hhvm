// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::io::Result;
use std::io::Write;

use bstr::BStr;
use bstr::BString;

use crate::BytesFormatter;

pub trait DisplayBytes {
    fn fmt(&self, fmt: &mut BytesFormatter<'_>) -> Result<()>;
}

impl<T: DisplayBytes + ?Sized> DisplayBytes for &T {
    fn fmt(&self, fmt: &mut BytesFormatter<'_>) -> Result<()> {
        <T as DisplayBytes>::fmt(self, fmt)
    }
}

#[macro_export]
macro_rules! display_bytes_using_display {
    ($name:ty) => {
        impl $crate::DisplayBytes for $name {
            fn fmt(&self, fmt: &mut $crate::BytesFormatter<'_>) -> std::io::Result<()> {
                use std::io::Write;
                write!(fmt, "{}", self)
            }
        }
    };
}

display_bytes_using_display!(str);
display_bytes_using_display!(String);
display_bytes_using_display!(i32);
display_bytes_using_display!(i64);
display_bytes_using_display!(isize);
display_bytes_using_display!(u32);
display_bytes_using_display!(u64);
display_bytes_using_display!(usize);

macro_rules! display_bytes_using_as_ref {
    ($name:ty) => {
        impl DisplayBytes for $name {
            fn fmt(&self, fmt: &mut BytesFormatter<'_>) -> Result<()> {
                fmt.write_all(self.as_ref())
            }
        }
    };
}

display_bytes_using_as_ref!(&[u8]);
display_bytes_using_as_ref!(BStr);
display_bytes_using_as_ref!(BString);

display_bytes_using_as_ref!(&[u8; 0]);
display_bytes_using_as_ref!(&[u8; 1]);
display_bytes_using_as_ref!(&[u8; 2]);
display_bytes_using_as_ref!(&[u8; 3]);
display_bytes_using_as_ref!(&[u8; 4]);
display_bytes_using_as_ref!(&[u8; 5]);
display_bytes_using_as_ref!(&[u8; 6]);
display_bytes_using_as_ref!(&[u8; 7]);
display_bytes_using_as_ref!(&[u8; 8]);
display_bytes_using_as_ref!(&[u8; 9]);
display_bytes_using_as_ref!(&[u8; 10]);
display_bytes_using_as_ref!(&[u8; 11]);
display_bytes_using_as_ref!(&[u8; 12]);
display_bytes_using_as_ref!(&[u8; 13]);
display_bytes_using_as_ref!(&[u8; 14]);
display_bytes_using_as_ref!(&[u8; 15]);
display_bytes_using_as_ref!(&[u8; 16]);

impl<T> DisplayBytes for std::borrow::Cow<'_, T>
where
    T: DisplayBytes + ToOwned + ?Sized,
    <T as ToOwned>::Owned: DisplayBytes,
{
    fn fmt(&self, fmt: &mut BytesFormatter<'_>) -> Result<()> {
        self.as_ref().fmt(fmt)
    }
}

impl<T: DisplayBytes + ?Sized> DisplayBytes for std::boxed::Box<T> {
    fn fmt(&self, fmt: &mut BytesFormatter<'_>) -> Result<()> {
        DisplayBytes::fmt(&**self, fmt)
    }
}
