There are two equality operators in Hack: `===` (recommended) and
`==`. They also have not-equal equivalents, which are `!==` and `!=`.

```Hack
1 === 2; // false
1 !== 2; // true
```

## `===` Equality

`===` compares objects by reference.

```Hack file:object.hack
class MyObject {}
```

```Hack file:object.hack
$obj = new MyObject();

// Different references aren't equal.
$obj === new MyObject(); // false

// The same reference is equal.
$obj === $obj; // true
```

`===` compares primitives types by value.

```Hack
1 === 1; // true
vec[1, 2] === vec[1, 2]; // true
```

Items of different primitive types are never equal.

```Hack
0 === null; // false
vec[1] === keyset[1]; // false

// Tip: if you want to compare an integer with a float,
// convert the integer value:
(float)1 === 1.0; // true
```

`vec`, `keyset`, `dict` and `shape` values are equal if their items
are `===` equal and if the items are in the same order.

```Hack
vec[1, 2] === vec[1, 2]; // true
vec[1] === vec[2]; // false

keyset[1, 2] === keyset[1, 2]; // true
keyset[1, 2] === keyset[2, 1]; // false

dict[0 => null, 1 => null] === dict[0 => null, 1 => null]; // true
dict[1 => null, 0 => null] === dict[0 => null, 1 => null]; // false

// Tip: Use Keyset\equal and Dict\equal if you
// want to ignore order:
Keyset\equal(keyset[1, 2], keyset[2, 1]); // true
Dict\equal(dict[1 => null, 0 => null], dict[0 => null, 1 => null]); // true
```

`!==` returns the negation of `===`.

```Hack
1 !== 2; // true
2 !== 2; // false
```

## `==` Equality

**If in doubt, prefer `===` equality.**

`==` compares objects by comparing each property, producing a
structural equality.

```Hack file:wrapper.hack
class MyWrapper {
  public function __construct(private mixed $m) {}
}
```

```Hack file:wrapper.hack
new MyWrapper(1) == new MyWrapper(1); // true
new MyWrapper(1) == new MyWrapper(2); // false
```

Items of different type are never equal with `==`, the same as `===`.

```Hack
1 == 1.0; // false
0 == null; // false
"" == 0; // false
```

`==` ignores the order when comparing `keyset`, `dict` and `shape`
values. Items within these collections are compared with `==`
recursively.

```Hack
keyset[1, 2] == keyset[2, 1]; // true
dict[1 => null, 2 => null] == dict[2 => null, 1 => null]; // true

// Note that vec comparisons always care about order.
vec[1, 2] == vec[2, 1]; // false
```

`!=` returns the negation of `==`.

```Hack
1 != 2; // true
1 != 1; // false
```
