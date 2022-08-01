/// Type A
pub type A = X;

/// Type B
/// is int
pub type B = X;

/// Type C has a fenced code block:
///
/// ```
/// function f(): int {
///   return 0;
/// }
/// ```
///
/// And an unfenced code block:
///
///     function g(): int {
///       return 0;
///     }
///
/// It should stay indented.
pub type C = X;

/** Type D has a multiline delimited comment:

```
function f(): int {
  return 0;
}
```

And an indented code block:

    ```
    function g(): int {
      return 0;
    }
    ```
*/
pub type D = X;

/// Records can have comments on the fields.
pub struct Record {
    /// The comments come after the field declaration in OCaml.
    pub foo: X,
    /// bar comment
    pub bar: X,
}

/// Variant types can have comments on each variant.
pub enum Variant {
    /// Again, the comments come after the variant declaration.
    /// Multiline comments are understood.
    Foo,
    /** Bar has a multiline delimited comment, even though it's
    unusual in Rust source. */
    Bar,
    /// Baz comment
    Baz { a: X, b: X },
    /// Qux is a struct-like variant with a long comment spanning
    /// multiple lines.
    #[rust_to_ocaml(prefix = "q_")]
    Qux { a: X, b: X },
}
