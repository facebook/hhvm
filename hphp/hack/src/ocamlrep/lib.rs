// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

/*!
OcamlRep is a framework for building and interpreting the in-memory
representation of OCaml values. This is useful for converting Rust values to
OCaml values and vice-versa, and for building and storing OCaml values off of
the OCaml runtime's garbage-collected heap.

OcamlRep provides a generic interface abstracting over the allocation of OCaml
values, allowing custom allocators to choose where values are allocated
(including directly onto the OCaml heap).

# Example: build an OCaml value ###############################################

This crate provides an arena-allocator which manages the memory in which
converted OCaml values are stored. When the arena is dropped, the values are
freed.

```rust
// Import the Allocator trait for access to its `add` method, which builds an
// OCaml-value representation of the argument (using `OcamlRep::to_ocamlrep`)
// and returns that value.
use ocamlrep::{Allocator, Arena, Value};

// The `ocamlrep` crate provides implementations of `OcamlRep` for builtin types
// like `Option`, `String`, integers, and tuples. This allows them to be
// converted to OCaml values using the Allocator trait (which Arena implements).
let tuple = (Some(42), String::from("a"));

// Allocate a chunk of Rust-managed backing storage for our OCaml value.
let arena = Arena::new();

// This value borrows the Arena, to ensure that we cannot continue to use it
// after the Arena has been freed. OcamlRep values do not borrow the Rust values
// they were converted from--the string "a" is copied into the Arena.
let ocamlrep_value: Value<'_> = arena.add(&tuple);

// We must now convert the value to a usize which can be handed over to the
// OCaml runtime. We must take care when doing this to ensure that our OCaml
// program doesn't use the value after the Arena is freed.
let ocaml_value: usize = ocamlrep_value.to_bits();
```

# Example: return an OCaml value to the OCaml runtime #########################

The value `ocaml_value` from the previous example is suitable to be handed to
the OCaml runtime. We might do so with an extern declaration like this:

```ocaml
external get_tuple : unit -> int option * string = "get_tuple"

let use_rust_tuple () =
  match get_tuple () with
  | (Some i, s) -> Printf.printf "%d, %s\n" i s
  | (None, s) -> Printf.printf "None, %s\n" s
```

We could provide this symbol from the Rust side like this:

```rust
#[no_mangle]
pub extern "C" fn get_tuple(_unit: usize) -> usize {
    use ocamlrep::{Allocator, Arena};
    let arena = Box::leak(Box::new(Arena::new()));
    let ocaml_tuple = arena.add(&(Some(42), String::from("a")));
    // This is safe because we leaked the Arena--no matter what we do with this
    // value on the OCaml side, we'll never use-after-free.
    ocaml_tuple.to_bits()
}
```

But this leaks memory. For small amounts of memory, or a short-lived process,
this might be fine. If not, we might choose another strategy...

# Example: lend an OCaml value to the OCaml runtime ###########################

Instead, we could register an OCaml callback which uses the value:

```ocaml
let use_tuple (tuple : int option * string) : unit =
  match tuple with
  | (Some i, s) -> Printf.printf "%d, %s\n" i s
  | (None, s) -> Printf.printf "None, %s\n" s

external make_and_use_tuple : unit -> unit = "make_and_use_tuple"

let () =
  Callback.register "use_tuple" use_tuple;
  make_and_use_tuple ()
```

And call into OCaml from Rust (using the `ocaml` crate) to hand over the value:

```rust
#[no_mangle]
pub extern "C" fn make_and_use_tuple(_unit: usize) -> usize {
    use ocamlrep::{Allocator, Arena};
    let arena = Arena::new();
    let tuple_ocamlrep = arena.add(&(Some(42), String::from("a")));

    let use_tuple = ocaml::named_value("use_tuple")
        .expect("use_tuple must be registered using Callback.register");

    // This is safe because we are passing the value to `use_tuple`, which
    // doesn't store the value and returns before we free the Arena.
    let ocaml_tuple: usize = tuple_ocamlrep.to_bits();
    use_tuple
        .call(ocaml::Value::new(ocaml_tuple))
        .expect("use_tuple must be a function");

    // Free the arena and return unit.
    ocaml::core::mlvalues::UNIT
}
```

# Example: pass an OCaml value to the OCaml runtime ###########################

The `ocamlrep_ocamlpool` crate provides an `Allocator` which builds values on
the OCaml runtime's garbage-collected heap. We can replace the memory-leaking
implementation from the [second example ("return an OCaml value to the OCaml
runtime")](#example-return-an-ocaml-value-to-the-ocaml-runtime) with one that
allows the OCaml value to be garbage-collected when no longer used:

```rust
#[no_mangle]
pub extern "C" fn get_tuple(_unit: usize) -> usize {
    ocamlrep_ocamlpool::to_ocaml(&(Some(42), String::from("a")))
}
```

The `ocamlrep_ocamlpool` crate provides a convenience macro for this use case:

```rust
use ocamlrep_ocamlpool::ocaml_ffi;

ocaml_ffi! {
    // This expands to code similar to the above definition of get_tuple.
    fn get_tuple() -> (Option<i32>, String) {
        (Some(42), String::from("a"))
    }
}
```

# Example: pass an OCaml value to Rust ########################################

An `Allocator` converts Rust values which implement the `OcamlRep` trait into
OCaml values using the method `OcamlRep::to_ocamlrep`. The `OcamlRep` trait also
provides a method `OcamlRep::from_ocamlrep` (and an FFI helper named
`OcamlRep::from_ocaml`) for conversion in the other direction.

If we call into Rust like this:

```ocaml
external use_tuple : int option * string -> unit = "use_tuple"

let () = use_tuple (Some 42, "a")
```

We could convert the tuple to a Rust value like this:

```rust
#[no_mangle]
pub extern "C" fn use_tuple(ocaml_tuple: usize) -> usize {
    // Import the OcamlRep trait to use its associated function `from_ocaml`.
    use ocamlrep::OcamlRep;
    // Safety: `ocaml_tuple` is a valid OCaml value allocated by the OCaml
    // runtime, all objects reachable from that value are also valid OCaml
    // values, and those objects cannot be concurrently modified while
    // `from_ocaml` runs. This is true because that graph of objects is owned by
    // the OCaml runtime, we didn't expose any of their pointers in any other
    // FFI functions, and the OCaml runtime is both single-threaded and
    // currently interrupted by an FFI call into this function.
    let tuple_result = unsafe { <(Option<i32>, String)>::from_ocaml(ocaml_tuple) };
    let tuple = tuple_result
        .expect("Expected a value of type `int option * string`, \
                 but got some other type or invalid UTF-8");
    println!("{:?}", tuple);
    ocaml::core::mlvalues::UNIT
}
```

The `ocaml_ffi!` macro in the `ocamlrep_ocamlpool` crate supports this use case:

```rust
use ocamlrep_ocamlpool::ocaml_ffi;

ocaml_ffi! {
    // This expands to code similar to the above definition of use_tuple.
    fn use_tuple(tuple: (Option<i32>, String)) {
        println!("{:?}", tuple)
    }
}
```

Note that the value returned by `from_ocaml` is owned--it is effectively a
deep clone of the OCaml value. For instance, the OCaml string "a" is copied into
a newly allocated Rust String--the Rust side does not need to worry about the
OCaml value being garbage collected.

Take care when using `from_ocaml`, `from_ocamlrep`, or `ocaml_ffi!` with types
containing `String`s. OCaml strings are not guaranteed to be UTF-8, so
`from_ocaml` may return an `Err` because a string was invalid UTF-8 rather than
because the OCaml code did not pass a value of the expected type (which should
be forbidden by the OCaml compiler, provided that the `external` declaration is
annotated with the correct type). If representing invalid UTF-8 is a
requirement, use `Vec<u8>` instead (an implementation of `OcamlRep` which
converts `Vec<u8>` to an OCaml `string` is provided).

# Example: implementing OcamlRep ##############################################

Writing a manual implementation of OcamlRep requires one to choose some type
with which their value should be represented in OCaml and write `to_ocamlrep`
and `from_ocamlrep` conversion functions which build and interpret the in-memory
representation of that type.

This crate provides implementations of OcamlRep for several std types, and
chooses these OCaml types to represent them with:

| Rust type              | OCaml type                              |
|------------------------|-----------------------------------------|
| `()`                   | `unit`                                  |
| `bool`                 | `bool`                                  |
| `usize`/`isize`        | `int`                                   |
| [`Option`][OptionRs]   | [`option`][OptionMl]                    |
| [`Result`][ResultRs]   | [`result`][ResultMl]                    |
| [`Vec`][Vec]           | [`list`][List]                          |
| [`String`][StringRs]   | [`string`][StringMl] (when valid UTF-8) |
| [`Vec<u8>`][Vec]       | [`string`][StringMl]                    |
| [`PathBuf`][PathBuf]   | [`string`][StringMl]                    |
| [`BTreeMap`][BTreeMap] | [`Map`][Map]                            |
| [`BTreeSet`][BTreeSet] | [`Set`][Set]                            |

[OptionRs]: https://doc.rust-lang.org/beta/std/option/enum.Option.html
[OptionMl]: https://caml.inria.fr/pub/docs/manual-ocaml/libref/Option.html
[ResultRs]: https://doc.rust-lang.org/std/result/enum.Result.html
[ResultMl]: https://caml.inria.fr/pub/docs/manual-ocaml/libref/Result.html
[Vec]: https://doc.rust-lang.org/std/vec/struct.Vec.html
[List]: https://caml.inria.fr/pub/docs/manual-ocaml/libref/List.html
[StringRs]: https://doc.rust-lang.org/std/string/struct.String.html
[StringMl]: https://caml.inria.fr/pub/docs/manual-ocaml/libref/String.html
[PathBuf]: https://doc.rust-lang.org/std/path/struct.PathBuf.html
[BTreeMap]: https://doc.rust-lang.org/std/collections/struct.BTreeMap.html
[Map]: https://caml.inria.fr/pub/docs/manual-ocaml/libref/Map.html
[BTreeSet]: https://doc.rust-lang.org/std/collections/struct.BTreeSet.html
[Set]: https://caml.inria.fr/pub/docs/manual-ocaml/libref/Set.html

See the [`impls`](../src/ocamlrep/impls.rs.html)
submodule for examples of conversions for std types, and
[Real World OCaml](https://v1.realworldocaml.org/v1/en/html/memory-representation-of-values.html)
for a description of the OCaml representation of values.

Since manually implementing OcamlRep is cumbersome and error-prone, the
`ocamlrep_derive` crate provides a procedural macro for deriving the OcamlRep
trait.

When derived for custom types like this:

```ocaml
use ocamlrep_derive::OcamlRep;

#[derive(OcamlRep)]
struct Foo {
    a: isize,
    b: Vec<bool>,
}

#[derive(OcamlRep)]
enum Fruit {
    Apple,
    Orange(isize),
    Pear { num: isize },
    Kiwi,
}
```

It produces implementations of OcamlRep which construct values belonging to
these roughly-equivalent OCaml types:

```ocaml
type foo = {
  a: int;
  b: bool list;
}

type fruit =
  | Apple
  | Orange of int
  | Pear of { num: int }
  | Kiwi
```

The `oxidize` program at hphp/hack/src/hh_oxidize will take OCaml source files
like the one above and generate Rust source files which define
"roughly-equivalent" types (like the Rust source above). These types will derive
an implementation of `OcamlRep` which will produce OCaml values belonging to the
corresponding type in the OCaml source file.
*/

mod arena;
mod block;
mod error;
mod impls;
mod value;

pub mod from;
pub mod ptr;
pub mod rc;
pub mod slab;

pub use arena::Arena;
pub use block::{Block, BlockBuilder};
pub use error::{FromError, SlabIntegrityError};
pub use impls::{bytes_from_ocamlrep, bytes_to_ocamlrep, str_from_ocamlrep, str_to_ocamlrep};
pub use value::{OpaqueValue, Value};

/// A data structure that can be converted to an equivalent OCaml value and
/// reconstructed from an OCaml value of the same (OCaml) type.
pub trait OcamlRep: Sized {
    /// Allocate an OCaml representation of `self` using the given Allocator.
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a>;

    /// Convert the given ocamlrep Value to a value of type `Self`, if possible.
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError>;

    /// Convert the given OCaml value to a value of type `Self`, if possible.
    ///
    /// # Safety
    ///
    /// The given value must be a valid OCaml value. All values reachable from
    /// the given value must be valid OCaml values. None of these values may be
    /// naked pointers. None of these values may be modified while `from_ocaml`
    /// is running.
    unsafe fn from_ocaml(value: usize) -> Result<Self, FromError> {
        Self::from_ocamlrep(Value::from_bits(value))
    }
}

/// An interface for allocating OCaml values in some allocator-defined memory region.
pub trait Allocator: Sized {
    /// Return a token which uniquely identifies this Allocator. The token must
    /// be unique (among all instances of all implementors of the Allocator
    /// trait), and an instance of an implementor of Allocator must return the
    /// same token throughout its entire lifetime.
    fn generation(&self) -> usize;

    /// Allocate a block with enough space for `size` fields, write its header,
    /// and return it.
    fn block_with_size_and_tag(&self, size: usize, tag: u8) -> BlockBuilder<'_>;

    /// Write the given value to the `index`th field of `block`.
    ///
    /// # Panics
    ///
    /// Panics if `index` is out of bounds for `block` (i.e., greater than or
    /// equal to the block's size).
    fn set_field<'a>(block: &mut BlockBuilder<'a>, index: usize, value: Value<'a>);

    #[inline(always)]
    fn block_with_size(&self, size: usize) -> BlockBuilder<'_> {
        self.block_with_size_and_tag(size, 0u8)
    }

    #[inline(always)]
    fn add<T: OcamlRep>(&self, value: &T) -> Value<'_> {
        value.to_ocamlrep(self)
    }
}
