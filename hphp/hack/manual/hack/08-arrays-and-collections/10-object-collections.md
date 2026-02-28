# Object Collections

The collection object types are `Vector`, `ImmVector`, `Map`, `ImmMap`,
`Set`, `ImmSet` and `Pair`. There are also a range of helper
interfaces, discussed below.

Hack Collection types are objects. They have reference semantics, so
they can be mutated.

Collections define a large number of methods you can use. They also
support array style syntax. Idiomatic Hack prefers array access syntax over methods, e.g.
`$v[0]` is better than `$v->at(0)`.

This page focuses on the core operations available, which all have
array access syntax. Consult the reference pages
(e.g. [Vector](/apis/Classes/HH/Vector/)) for the full list of
methods.

## `Vector` and `ImmVector`

*Where possible, we recommend using `vec` instead.*

A `Vector` is a mutable ordered data structure. It is
created with the `Vector {}` syntax.

```hack
// Creating a Vector.
function get_items(): Vector<string> {
  $items = Vector {'a', 'b', 'c'};
  return $items;
}
```

`Vector`s can be accessed with the following syntax.

```hack
$items = Vector {'a', 'b', 'c'};

// Accessing items by index.
$items[0]; // 'a'
$items[3]; // throws OutOfBoundsException

// Accessing items that might be out-of-bounds.
idx($items, 0); // 'a'
idx($items, 3); // null
idx($items, 3, 'default'); // 'default'

// Modifying items. This mutates the Vector in place.
$items[0] = 'xx'; // Vector {'xx', 'b', 'c'}
$items[] = 'd'; // Vector {'xx', 'b', 'c', 'd'}

// Getting the length.
C\count($items); // 4

// Iterating.
foreach ($items as $item) {
  echo $item;
}
// Iterating with the index.
foreach ($items as $index => $item) {
  echo $index; // e.g. 0
  echo $item; // e.g. 'a'
}

// Equality checks compare references.
$items === $items; // true
Vector {} === Vector {}; // false

// Converting from an Iterable.
new Vector(vec[1, 2]); // Vector {1, 2}
new Vector(Set {1, 2}); // Vector {1, 2}
new Vector(dict['key1' => 'value1']); // Vector {'value1'}

// Type checks
$items is Vector<_>; // true
```

`ImmVector` is an immutable version of `Vector`.

``` Hack
// Creating an ImmVector.
function get_items(): ImmVector<string> {
  $items = ImmVector {'a', 'b', 'c'};
  return $items;
}
```

## `Set` and `ImmSet`

*Where possible, we recommend using `keyset` instead.*

A `Set` is a mutable, ordered, data structure without duplicates. It
is created with the `Set {}` syntax.

A `Set` can only contain `string` or `int` values.

```hack
// Creating a Set.
function get_items(): Set<string> {
  $items = Set {'a', 'b', 'c'};
  return $items;
}
```

`Set`s can be accessed with the following syntax.

```hack
$items = Set {'a', 'b', 'c'};

// Checking if a Set contains a value.
C\contains_key($items, 'a'); // true

// Modifying items. This mutates the Set in place.
$items[] = 'd'; // Set {'a', 'b', 'c', 'd'}
$items[] = 'a'; // Set {'a', 'b', 'c', 'd'}

// Getting the length.
C\count($items); // 4

// Iterating.
foreach ($items as $item) {
  echo $item;
}

// Equality checks compare references.
$items === $items; // true
Set {} === Set {}; // false

// Converting from an Iterable.
new Set(vec[1, 2, 1]); // Set {1, 2}
new Set(Vector {20, 21}); // Set {20, 21}
new Set(dict['key1' => 'value1']); // Set {'value1'}

// Type checks.
$items is Set<_>; // true
```

`ImmSet` is an immutable version of `Set`.

``` Hack
// Creating an ImmSet.
function get_items(): ImmSet<string> {
  $items = ImmSet {'a', 'b', 'c'};
  return $items;
}
```


## `Map` and `ImmMap`

*Where possible, we recommend using `dict` instead.*

A `Map` is a mutable, ordered, key-value data structure. It is
created with the `Map {}` syntax.

Keys must be `string`s or `int`s.

```hack
// Creating a Map.
function get_items(): Map<string, int> {
  $items = Map {'a' => 1, 'b' => 3};
  return $items;
}
```

`Map`s can be accessed with the following syntax.

```hack
$items = Map {'a' => 1, 'b' => 3};

// Accessing items by key.
$items['a']; // 1
$items['z']; // throws OutOfBoundsException

// Accessing keys that may be absent.
idx($items, 'a'); // 1
idx($items, 'z'); // null
idx($items, 'z', 'default'); // 'default'

// Modifying items. This mutates the Map in place.
$items['a'] = 42; // Map {'a' => 42, 'b' => 3}
$items['z'] = 100; // Map {'a' => 42, 'b' => 3, 'z' => 100}

// Getting the keys.
Vec\keys(Map {'a' => 1, 'b' => 3}); // vec['a', 'b']

// Getting the values.
vec(Map {'a' => 1, 'b' => 3}); // vec[1, 3]

// Getting the length.
C\count($items); // 3

// Iterating values.
foreach ($items as $value) {
  echo $value;
}
// Iterating keys and values.
foreach ($items as $key => $value) {
  echo $key;
  echo $value;
}

// Equality checks compare references.
$items === $items; // true
Map {} === Map {}; // false

// Converting from an Iterable.
new Map(dict['key1' => 'value1']); // Map { 'key1' => 'value1'}
new Map(vec['a', 'b']); // Map {0 => 'a', 1 => 'b'}

// Type checks
$items is Map<_, _>; // true
```

`ImmMap` is an immutable version of `Map`.

``` Hack
// Creating an ImmMap.
function get_items(): ImmMap<string, int> {
  $items = ImmMap {'a' => 1, 'b' => 3};
  return $items;
}
```

## `Pair`

*Where possible, we recommend using `tuple` instead.*

A `Pair` is an immutable data structure with two items. It is
created with the `Pair {}` syntax.

``` Hack
function get_items(): Pair<int, string> {
  $items = Pair {42, 'foo'};
  return $items;
}
```

`Pair`s can be accessed with the following syntax.

```hack
$items = Pair {42, 'foo'};

// Destructuring a Pair value.
list($x, $y) = $items; // $x: 42, $y: 'foo'

// Accessing elements by index.
$items[0]; // 42
$items[1]; // 'foo'
```

## Interfaces

Hack Collections implement a range of helper interfaces, so your code
can handle multiple Hack Collection types.

If you want to handle both Hack arrays and Hack Collections, use
`Traversable`. If you want to read and write a Hack Collection, use
`Collection`. If you only want to read a Hack Collection, use
`ConstCollection`.

The `ConstCollection` interface represents Hack Collections that be
can read from.

```text
 ConstCollection
  +-- ConstVector
  |    +-- ImmVector
  |    +-- Pair
  +-- ConstSet
  |    +-- ImmSet
  +-- ConstMap
  |    +-- ImmMap
  +-- Collection
       +-- MutableVector
       |    +-- Vector
       +-- MutableMap
       |    +-- Map
       +-- MutableSet
            +-- Set
```

The `OutputCollection` interface represents Hack Collections that be
can written to.

```text
 OutputCollection
  +-- Collection
       +-- MutableVector
       |    +-- Vector
       +-- MutableMap
       |    +-- Map
       +-- MutableSet
            +-- Set
```
