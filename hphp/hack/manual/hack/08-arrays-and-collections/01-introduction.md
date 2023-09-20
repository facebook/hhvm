Hack includes diverse range of array-like data structures.

Hack arrays are value types for storing iterable data. The types
available are [`vec`](/hack/arrays-and-collections/vec-keyset-and-dict#vec), [`dict`](/hack/arrays-and-collections/vec-keyset-and-dict#dict) and [`keyset`](/hack/arrays-and-collections/vec-keyset-and-dict#keyset). **When in doubt, use Hack
arrays**.

Hack collections are **deprecated** object types for storing iterable data. The types
available include `Vector`, `Map`, `Set`, `Pair` and helper
interfaces.

## Quickstart
You can create Hack arrays as follows:

```Hack
$v = vec[2, 1, 2];

$k = keyset[2, 1];

$d = dict['a' => 1, 'b' => 3];
```

## The Hack Standard Library
There are many helpful functions in the `C`, `Vec`, `Keyset` and `Dict`
namespaces, which are a part of the [Hack Standard Library (HSL)](/hsl/reference).

For more information on included HSL namespaces, see [Hack Standard Library: Namespaces](/hack/getting-started/the-hack-standard-library#hsl-namespaces).

```Hack
// The C namespace contains generic functions that are relevant to
// all array and collection types.
C\count(vec[]); // 0
C\is_empty(keyset[]); // true

// The Vec, Keyset and Dict namespaces group functions according
// to their return type.
Vec\keys(dict['x' => 1]); // vec['x']
Keyset\keys(dict['x' => 1]); // keyset['x']

Vec\map(keyset[1, 2], $x ==> $x + 1); // vec[2, 3]
```

## Arrays Cheat Sheet

| Operation| `vec`    | `dict`   | `keyset` |
|----------|----------|----------|----------|
| Initialize empty                             | `$v = vec[];`                | `$d = dict[];`                 | `$k = keyset[];`               |
| Literal                                      | `$v = vec[1, 2, 3];`         | `$d = dict['foo' => 1];`       | `$k = keyset['foo', 'bar'];`   |
| From Another Container*                      | `$v = vec($container);`      | `$d = dict($keyed_container);` | `$k = keyset($container);`     |
| Keys from Container*                         | `$v = Vec\keys($container);` | N/A                            | `$k = Keyset\keys($container);`|
| Add Elements                                 | `$v[] = 4;`                  | `$d['baz'] = 2;`               | `$k[] = 'baz';`                |
| Bulk Add Elements                            | `$v = Vec\concat($t1, $t2)`  | `$d = Dict\merge($kt1, $kt2)`  | `$k = Keyset\union($t1, $t2)`  |
| Remove Elements                              | Remove-at-index is unsupported; `Vec\drop($v,$n)`, `Vec\take($v,$n)`; `$first=C\pop_front(inout $x)`, `$last=C\pop_back(inout $x)` | `unset($d['baz']);`  | `unset($k['baz']);`|
| Key Existence                                | `C\contains_key($v, 1)`      | `C\contains_key($d, 'foo')`    | `C\contains_key($k, 'foo')`    |
| Value Existence                              | `C\contains($v, 3)`          | `C\contains($d, 2)`            | Use `C\contains_key($k, 'foo')`|
| Equality (Order-Dependent)                   | `$v1 === $v2`                | `$d1 === $d2`                  | `$k1 === $k2`                  |
| Equality (Order-Independent)                 | N/A                          | `Dict\equal($d1, $d2)`         | `Keyset\equal($k1, $k2)`       |
| Count Elements (i.e., length, size of array) | `C\count($v)`                | `C\count($d)`                  | `C\count($k)`                  |
| Type Signature                               | `vec<Tv>`                    | `dict<Tk, Tv>`                 | `keyset<Tk>`                   |
| Type Refinement                              | `$v is vec<_>`               | `$d is dict<_, _>`             | `$k is keyset<_>`              |
| `Awaitable` Consolidation                    | `Vec\from_async($v)`         | `Dict\from_async($d)`          | `Keyset\from_async($x)`        |

\* `$container` can be a Hack Array or Hack Collection

## Arrays Conversion Cheat sheet

Prefer to use Hack arrays whenever possible. When interfacing with legacy APIs that expect older Containers, it may be easier to convert. Here's how:

| Converting | To `Vector`| To `Map`   | To `Set`   |
|------------|------------|------------|------------|
| `dict`        | N/A                      | `new Map($d)` | N/A                   |
| `dict` keys   | `Vector::fromKeysOf($d)` | N/A           | `Set::fromKeysOf($d)` |
| `dict` values | `new Vector($d)`         | N/A           | `new Set($d)`         |
| `vec`         | `new Vector($v)`         | `new Map($v)` | `new Set($v)`         |
| `keyset`      | `new Vector($k)`         | `new Map($k)` | `new Set($k)`         |
