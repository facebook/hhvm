**`varray`, `darray` and `varray_or_darray` are legacy value types for
storing iterable data.**. They are also called 'PHP arrays' and will
eventually be removed.

PHP arrays are immutable value types, just like Hack arrays. Unlike
Hack arrays, they include legacy behaviors from PHP that can hide
bugs.

For example, in HHVM 4.36, invalid array keys are accepted and
silently coerced to an `arraykey`.

```Hack no-extract
$x = dict[false => 123];
var_dump(array_keys($x)[0]);
// int(0), not `bool(false)`
```

## `varray`

#### Use Hack Arrays

As of [HHVM 4.103](https://hhvm.com/blog/2021/03/31/hhvm-4.103.html), `varray` is aliased to `vec`. Use [`vec`](https://docs.hhvm.com/hack/arrays-and-collections/hack-arrays#vec).

#### Working with varrays

A `varray` is an ordered, iterable data structure.

```Hack no-extract
// Creating a varray.
function get_items(): varray<string> {
  $items = vec['a', 'b', 'c'];
  return $items;
}

// Accessing items by index.
$items[0]; // 'a'
$items[3]; // throws OutOfBoundsException

// Accessing items that might be out-of-bounds.
idx($items, 0); // 'a'
idx($items, 3); // null
idx($items, 3, 'default'); // 'default'

// Modifying items. These operations set $items
// to a modified copy, and do not modify the original value.
$items[0] = 'xx'; // vec['xx', 'b', 'c']
$items[] = 'd'; // vec['xx', 'b', 'c', 'd']

// Getting the length.
C\count($items); // 4

// Iterating.
foreach ($items as $item) {
  echo $item;
}
// Iterating with the index.
foreach ($items as $index => $item) {
  echo $index; // e.g. 0
  echo $item;  // e.g. 'a'
}

// Equality checks.
vec[1] === vec[1]; // true
vec[1, 2] === vec[2, 1]; // false

// Converting from an Iterable.
varray(vec[10, 11]); // vec[10, 11]
varray(keyset[10, 11]); // vec[10, 11]
```

## `darray`

#### Use Hack Arrays

As of [HHVM 4.103](https://hhvm.com/blog/2021/03/31/hhvm-4.103.html), `darray` is aliased to `dict`. Use [`dict`](https://docs.hhvm.com/hack/arrays-and-collections/hack-arrays#dict).

#### Working with darrays

A `darray` is an ordered key-value data structure.

```Hack no-extract
// Creating a darray.
function get_items(): darray<string, int> {
  $items = dict['a' => 1, 'b' => 3];
  return $items;
}

// Accessing items by key.
$items['a']; // 1
$items['foo']; // throws OutOfBoundsException

// Accessing keys that may be absent.
idx($items, 'a'); // 1
idx($items, 'z'); // null
idx($items, 'z', 'default'); // 'default'

// Inserting, updating or removing values in a darray. These operations
// set $items to a modified copy, and do not modify the original value.
$items['a'] = 42; // dict['a' => 42, 'b' => 3]
$items['z'] = 100; // dict['a' => 42, 'b' => 3, 'z' => 100]
unset($items['b']); // dict['a' => 42, 'z' => 100]

// Getting the keys.
Vec\keys(dict['a' => 1, 'b' => 3]); // vec['a', 'b']

// Getting the values.
vec(dict['a' => 1, 'b' => 3]); // vec[1, 3]

// Getting the length.
C\count($items); // 2

// Checking if a dict contains a key or value.
C\contains_key($items, 'a'); // true
C\contains($items, 3); // true

// Iterating values.
foreach ($items as $value) {
  echo $value; // e.g. 1
}
// Iterating keys and values.
foreach ($items as $key => $Value) {
  echo $key;   // e.g. 'a'
  echo $value; // e.g. 1
}

// Equality checks. === returns false if the order does not match.
dict[] === dict[]; // true
dict[0 => 10, 1 => 11] === dict[1 => 11, 0 => 10]; // false

// Converting from an Iterable.
darray(vec['a', 'b']); // dict[0 => 'a', 1 => 'b']
darray(Map {'a' => 5}); // dict['a' => 5]
```

## `varray_or_darray`

A `varray_or_darray` is type that can be either a `varray` or
`darray`. It exists to help gradually migrate code to more specific
types, and should be avoided when possible.

```Hack
function get_items(bool $b): varray_or_darray<int, string> {
  if ($b) {
    return vec['a', 'b'];
  } else {
    return dict[5 => 'c'];
  }
}
```

## Runtime options

In HHVM version 4.62 and earlier, by default, `varray` and `darray` are interchangeable at runtime; this can be changed, as can some legacy PHP array behaviors, depending on the HHVM version.

The available runtime options change frequently; to get an up-to-date list, search `ini_get_all()` for settings beginning with `hhvm.hack_arr`; in general:
- a value of `0` means no logging
- `1` means a warning or notice is raised
- `2` means a recoverable error is raised

The `hhvm.hack_arr_compat_notices` option must be set to true for any of the `hhvm.hack_arr_` options to have an effect.

Individual runtime settings are documented [here](/hack/built-in-types/darray-varray-runtime-options.md).
