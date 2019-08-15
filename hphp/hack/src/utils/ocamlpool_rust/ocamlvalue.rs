// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

extern crate ocaml;

use crate::utils::*;
use ocaml::core::mlvalues::{empty_list, Value};
use std::borrow::Cow;

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

impl Ocamlvalue for bool {
    fn ocamlvalue(&self) -> Value {
        usize_to_ocaml(*self as usize)
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

impl Ocamlvalue for i8 {
    fn ocamlvalue(&self) -> Value {
        usize_to_ocaml(*self as usize)
    }
}

impl Ocamlvalue for isize {
    fn ocamlvalue(&self) -> Value {
        ((*self << 1) + 1) as usize
    }
}
