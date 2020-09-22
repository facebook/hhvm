// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ocamlrep_custom::caml_serialize_default_impls;
use ocamlrep_custom::{CamlSerialize, Custom};
use ocamlrep_ocamlpool::ocaml_ffi;

use std::cell::RefCell;
use std::convert::TryInto;
use std::io::Write;
use std::rc::Rc;

struct DropTest(Rc<RefCell<bool>>);

impl CamlSerialize for DropTest {
    caml_serialize_default_impls!();
}

struct DropTestCell(Rc<RefCell<bool>>);

impl CamlSerialize for DropTestCell {
    caml_serialize_default_impls!();
}

impl DropTest {
    pub fn new() -> Self {
        Self(Rc::new(RefCell::new(false)))
    }

    pub fn cell(&self) -> Rc<RefCell<bool>> {
        self.0.clone()
    }
}

impl Drop for DropTest {
    fn drop(&mut self) {
        *self.0.borrow_mut() = true;
    }
}

ocaml_ffi! {
  fn test_custom_drop_test_new() -> Custom<DropTest> {
    Custom::from(DropTest::new())
  }

  fn test_custom_drop_test_custom_ref_count(x: Custom<DropTest>) -> usize {
    let w = Rc::downgrade(x.inner());
    drop(x);
    w.strong_count()
  }

  fn test_custom_drop_test_get_cell(x: Custom<DropTest>) -> Custom<DropTestCell> {
    Custom::from(DropTestCell(x.cell()))
  }

  fn test_custom_drop_test_cell_is_dropped(x: Custom<DropTestCell>) -> bool {
    *x.0.borrow()
  }
}

struct BoxedInt(isize);

impl CamlSerialize for BoxedInt {
    caml_serialize_default_impls!();

    fn serialize(&self) -> Vec<u8> {
        let mut buffer = Vec::new();
        buffer.write_all(&self.0.to_be_bytes()).unwrap();
        buffer
    }

    fn deserialize(buffer: &[u8]) -> Self {
        let i = isize::from_be_bytes(buffer[0..std::mem::size_of::<isize>()].try_into().unwrap());
        BoxedInt(i)
    }
}

ocaml_ffi! {
  fn test_custom_boxed_int_register() {
    // Safety: called from OCaml in a single-threaded context.
    unsafe {
      BoxedInt::register();
    }
  }

  fn test_custom_boxed_int_new(x: isize) -> Custom<BoxedInt> {
    Custom::from(BoxedInt(x))
  }

  fn test_custom_boxed_int_equal(x: Custom<BoxedInt>, y: Custom<BoxedInt>) -> bool {
    x.0 == y.0
  }
}
