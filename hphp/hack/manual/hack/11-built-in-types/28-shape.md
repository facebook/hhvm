A shape is a lightweight type with named fields. It's similar to
structs or records in other programming languages.

```Hack
$my_point = shape('x' => -3, 'y' => 6, 'visible' => true);
```

## Shape Values

A shape is created with the `shape` keyword, with a series of field
names and values.

``` Hack
$server = shape('name' => 'db-01', 'age' => 365);

$empty = shape();
```

Shape fields are accessed with array indexing syntax, similar to
`dict`. Note that field names must be string literals.

```Hack no-extract
// OK.
$n = $server['name'];

// Not OK (type error).
$field = 'name';
$n = $server[$field];
```

Shapes are copy-on-write.

``` Hack
$s1 = shape('name' => 'db-01', 'age' => 365);
$s2 = $s1;

$s2['age'] = 42;
// $s1['age'] is still 365.
```

A shape can be constructed incrementally. The type checker will infer
a different type after each assignment.

``` Hack
// $s has type shape().
$s = shape();

// $s now has type shape('name' => string).
$s['name'] = 'db-01';

// $s now has type shape('name' => string, 'age' => int).
$s['age'] = 365;
```

Shapes have the same runtime representation as `darray`, although this
is considered an implementation detail. This representation means that
shape order is observable.

``` Hack
$s1 = shape('name' => 'db-01', 'age' => 365);
$s2 = shape('age' => 365, 'name' => 'db-01');

$s1 === $s2; // false
```

## Shape Types

Shape type declarations use a similar syntax to values.

``` Hack
function takes_server(shape('name' => string, 'age' => int) $s): void {
  // ...
}
```

Unlike classes, declaring a shape type is optional. You can start
using shapes without defining any types.

``` Hack no-extract
function uses_shape_internally(): void {
  $server = shape('name' => 'db-01', 'age' => 365);
  print_server_name($server['name']);
  print_server_age($server['age']);
}
```

For large shapes, it is often convenient to define a type alias.  This is useful because it promotes code re-use and when the same type is being used, and provides a descriptive name for the type.

```Hack
type Server = shape('name' => string, 'age' => int);

// Equivalent to the previous takes_server function.
function takes_server(Server $s): void {
  // ...
  return;
}
```

Any shape value that has all of the required fields (and no undefined fields - unless the shape permits them) is considered a value of type `Server`; the type is not specified when creating the value.

```Hack error
function takes_server(Server $s): void {
  return;
}

function test(): void {
  $args = shape('name' => 'hello', 'age' => 10);
  takes_server($args); // no error

  $args = shape('name' => null, 'age' => 10);
  takes_server($args); // type error: field type mismatch

  $args = shape('name' => 'hello', 'age' => 10, 'error' => true);
  takes_server($args); // type error: extra field
}
```

Since shapes are copy-on-write, updates can change the type.

```Hack
// $s has type shape('name' => string, 'age' => int).
$s = shape('name' => 'db-01', 'age' => 365);

// $s now has type shape('name' => string, 'age' => string).
$s['age'] = '1 year';
```

**Two shapes have the same type if they have the same fields and
types**. This makes shapes convenient to create, but can cause
surprises. This is called 'structural subtyping'.

```Hack
type Server = shape('name' => string, 'age' => int);
type Pet = shape('name' => string, 'age' => int);

function takes_server(Server $_): void {}

function takes_pet(Pet $p): void {
  // No error here.
  takes_server($p);
}
```

## Open and Closed Shapes

Normally, the type checker will enforce that you provide exactly the
fields specified. This is called a 'closed shape'.

```Hack error
function takes_named(shape('name' => string) $_): void {}

function demo(): void {
  takes_named(shape('name' => 'db-01', 'age' => 365)); // type error
}
```

Shape types may include `...` to indicate that additional fields are
permitted. This is called an 'open shape'.

```Hack
function takes_named(shape('name' => string, ...) $_): void {}

// OK.
function demo(): void {
  takes_named(shape('name' => 'db-01', 'age' => 365));
}
```

To access the additional fields in an open shape, you can use
`Shapes::idx`.

```Hack
function takes_named(shape('name' => string, ...) $n): void {
  // The value in the shape, or null if field is absent.
  $nullable_age = Shapes::idx($n, 'age');

  // The value in the shape, or 0 if field is absent.
  $age_with_default = Shapes::idx($n, 'age', 0);
}

```

## Optional Fields

A shape type may declare fields as optional.

```Hack
function takes_server(shape('name' => string, ?'age' => int) $s): void {
  $age = Shapes::idx($s, 'age', 0);
}

function example_usage(): void {
  takes_server(shape('name' => 'db-01', 'age' => 365));
  takes_server(shape('name' => 'db-02'));
}
```

`takes_server` takes a closed shape, so any additional fields will be
an error. The `age` field is optional though.

Optional fields can be tricky to reason about, so your code may be
clearer with nullable fields or open shapes.

```Hack
function takes_server2(shape('name' => string, 'age' => ?int) $s): void {
  $age = $s['age'] ?? 0;
}

function takes_server3(shape('name' => string, ...) $s): void {
  $age = Shapes::idx($s, 'age', 0) as int;
}
```

## Type Enforcement

HHVM will check that arguments are shapes, but it will not deeply
check fields.

```Hack error
// This produces a typehint violation at runtime.
function returns_int_instead(): shape('x' => int) {
  return 1;
}

// No runtime error.
function returns_wrong_shape(): shape('x' => int) {
  return shape('y' => 1);
}
```

## Converting Shapes

Converting shapes to containers is strongly discouraged, however is necessary, this can be done with `Shapes::toDict()`.

On older versions of HHVM, Shapes can also be converted to darrays with `Shapes::toArray()`; this should be avoided in new code, as darrays are currently an alias for the dict type, and will be removed from the language.

## Limitations

Some limitations of shapes include only being able to index it using literal expressions (you can't index on a shape using a variable or dynamically formed string, for example), or to provide run-time typechecking, because it is actually just a `dict` at runtime (or `darray` on older versions).
