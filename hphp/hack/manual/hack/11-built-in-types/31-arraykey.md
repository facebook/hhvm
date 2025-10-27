# Arraykey

The type `arraykey` can represent any integer or string value.  For example:

```hack
function process_key(arraykey $p): void {
  if ($p is int) {
    // we have an int
  } else {
    // we have a string
  }
}
```

Values of array or collection type can be indexed by `int` or `string`. Suppose, for example, an operation was performed on an array
to extract the keys, but we didn't know the type of the key. As such, we are left with using `mixed` (which is way too loose) or doing
some sort of duplicative code. Instead, we can use `arraykey`.

See the discussion of [type refinement](/hack/types/type-refinement).
