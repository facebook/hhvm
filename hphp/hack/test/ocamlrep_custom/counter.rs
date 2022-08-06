use std::cell::Cell;

use ocamlrep_custom::caml_serialize_default_impls;
use ocamlrep_custom::CamlSerialize;
use ocamlrep_custom::Custom;
use ocamlrep_ocamlpool::ocaml_ffi;

pub struct Counter(Cell<isize>);

impl CamlSerialize for Counter {
    caml_serialize_default_impls!();
}

ocaml_ffi! {
    fn counter_new() -> Custom<Counter> {
        Custom::from(Counter(Cell::new(0)))
    }

    fn counter_inc(counter: Custom<Counter>) -> Custom<Counter> {
        counter.0.set(counter.0.get() + 1);
        counter
    }

    fn counter_read(counter: Custom<Counter>) -> isize {
        counter.0.get()
    }
}
