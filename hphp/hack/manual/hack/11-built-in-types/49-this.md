# This

The type name `this` refers to *the current class type at run time*. As such, it can only be used from within a class, an interface, or
a trait. (The type name `this` should not be confused with [`$this`](/hack/source-code-fundamentals/names), which refers to *the current
instance*, whose type is `this`.)  For example:

```hack
interface I1 {
  abstract const type T1 as arraykey;
  public function get_ID(): this::T1;
}
```

Here, the function `get_ID` returns a value whose type is based on the type of the class that implements this interface type.

Strictly speaking, `this` is *not* a new type name, just an alias for an existing one.
