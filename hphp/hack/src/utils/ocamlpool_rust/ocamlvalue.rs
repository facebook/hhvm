// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

extern crate ocaml;

use crate::utils::*;

use ocaml::core::mlvalues::{empty_list, Value, UNIT};
use std::borrow::Cow;
use std::path::PathBuf;
use std::rc::Rc;
use std::result::Result;

const DOUBLE_TAG: u8 = 253;

pub trait Ocamlvalue {
    fn ocamlvalue(&self) -> Value;
}

impl<T: Ocamlvalue> Ocamlvalue for std::boxed::Box<T> {
    fn ocamlvalue(&self) -> Value {
        self.as_ref().ocamlvalue()
    }
}

impl<T: Ocamlvalue> Ocamlvalue for std::vec::Vec<T> {
    fn ocamlvalue(&self) -> Value {
        let mut res = empty_list();

        for v in self.iter().rev() {
            res = caml_tuple(&[v.ocamlvalue(), res])
        }
        res
    }
}

impl<T: Ocamlvalue> Ocamlvalue for Option<T> {
    fn ocamlvalue(&self) -> Value {
        match self {
            None => usize_to_ocaml(0),
            Some(x) => caml_tuple(&[x.ocamlvalue()]),
        }
    }
}

impl Ocamlvalue for String {
    fn ocamlvalue(&self) -> Value {
        str_to_ocaml(self.as_bytes())
    }
}

impl Ocamlvalue for Cow<'static, str> {
    fn ocamlvalue(&self) -> Value {
        str_to_ocaml(self.as_bytes())
    }
}

impl Ocamlvalue for &str {
    fn ocamlvalue(&self) -> Value {
        str_to_ocaml(self.as_bytes())
    }
}

impl Ocamlvalue for bool {
    fn ocamlvalue(&self) -> Value {
        usize_to_ocaml(*self as usize)
    }
}

impl Ocamlvalue for () {
    fn ocamlvalue(&self) -> Value {
        UNIT
    }
}

impl<T1: Ocamlvalue, T2: Ocamlvalue> Ocamlvalue for (T1, T2) {
    fn ocamlvalue(&self) -> Value {
        caml_tuple(&[self.0.ocamlvalue(), self.1.ocamlvalue()])
    }
}

impl<T1: Ocamlvalue, T2: Ocamlvalue, T3: Ocamlvalue> Ocamlvalue for (T1, T2, T3) {
    fn ocamlvalue(&self) -> Value {
        caml_tuple(&[
            self.0.ocamlvalue(),
            self.1.ocamlvalue(),
            self.2.ocamlvalue(),
        ])
    }
}

impl<T1: Ocamlvalue, T2: Ocamlvalue, T3: Ocamlvalue, T4: Ocamlvalue> Ocamlvalue
    for (T1, T2, T3, T4)
{
    fn ocamlvalue(&self) -> Value {
        caml_tuple(&[
            self.0.ocamlvalue(),
            self.1.ocamlvalue(),
            self.2.ocamlvalue(),
            self.3.ocamlvalue(),
        ])
    }
}

impl Ocamlvalue for i8 {
    fn ocamlvalue(&self) -> Value {
        usize_to_ocaml(*self as usize)
    }
}

impl Ocamlvalue for usize {
    fn ocamlvalue(&self) -> Value {
        usize_to_ocaml(*self as usize)
    }
}

impl Ocamlvalue for isize {
    fn ocamlvalue(&self) -> Value {
        ((*self << 1) + 1) as usize
    }
}

impl Ocamlvalue for u64 {
    fn ocamlvalue(&self) -> Value {
        ((*self << 1) + 1) as usize
    }
}

impl Ocamlvalue for f64 {
    fn ocamlvalue(&self) -> Value {
        caml_block(DOUBLE_TAG, &[(*self).to_bits() as usize])
    }
}

impl<T: Ocamlvalue> Ocamlvalue for Rc<T> {
    fn ocamlvalue(&self) -> Value {
        self.as_ref().ocamlvalue()
    }
}

impl Ocamlvalue for PathBuf {
    fn ocamlvalue(&self) -> Value {
        self.to_str().unwrap().ocamlvalue()
    }
}

impl<T: Ocamlvalue, E: Ocamlvalue> Ocamlvalue for Result<T, E> {
    fn ocamlvalue(&self) -> Value {
        match self {
            Result::Ok(t) => caml_block(0, &[t.ocamlvalue()]),
            Result::Err(e) => caml_block(1, &[e.ocamlvalue()]),
        }
    }
}
