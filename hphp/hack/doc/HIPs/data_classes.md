Title: Data Classes
Status: Draft

## The Problem

The Hack runtime does not know which fields are defined in an arbitrary
shape, and does not enforce them. It also cannot reorder shape fields, and
cannot store them contiguously.

This forces the runtime to use data representations that are slower
and use more memory.

By defining a new data type with runtime enforcement, we can provide a
high performance data structure where HHVM knows the fields.

## Design Summary

Data classes are high-performance immutable classes, enforced at
runtime, with syntactic sugar for creation and updates.

### Basic Usage

Data classes are defined using a syntax similar to other forms of
classes.

```
data class User {
  string $name;
  bool $admin;
  ?string $email = null;
}
```

Data classes live in the type namespace, so they must have a name
distinct from other classes.

Data classes are instantiated with `[]`. This enables HackC to produce
different bytecodes for normal class instantiation and data class
instantiation.

```
$p = Person[name = "wilfred", admin = false];
```

Properties on data classes are accessed with `->`, consistent with
normal classes.

```
// Access
$n = $p->name;
```

Data classes are immutable. To create a new data class with slightly
modified fields, use `...` splat syntax.

```
$p2 = Person[...$p, name = "anonymous"];
```

### Inheritance

Data classes can inherit from other data classes with single
inheritance. Child classes may only add new fields.

```
abstract data class Named {
  string $name;
}

data class User extends Named {
  bool $admin;
}
```

Only abstract data classes may have children. Abstract data classes
may not be instantiated.

### Runtime Type Information

Data classes have distinct type tags. This enables HHVM to enforce
the names and types of properties. These runtime tags may also be
inspected with `is` and `as`, consistent with normal classes.

```
$p is Person; // true
```

This enables runtime type enforcement, unlike shapes.

```
// Type checker error and runtime error.
$p = Person[...$p, name = 123];
```

Function boundaries can also enforce types at runtime.

```
data class Server {
  string $name;
}
data class Person {
  string $name;
}

// Type checker error and runtime error.
takes_person(Server[server = 'db-001']);
```

This is nominal subtyping. Note that shapes have structural subtyping,
so `takes_person_shape(shape('name' => 'db-001'))` produces no errors.

## Release and Migration

HHVM will have a feature flag to prevent people using data
classes.

This enables us to ship frontend support (parser, codegen, type
checker) for data classes without accidentally allowing users to use this.

This does not replace any existing Hack features, so we do not
anticipate large scale codemods. Hotspots in performance sensitive
code will be worth migrating. Users may also prefer to migrate shape
code that doesn't heavily rely on structural subtyping.

## Implementation Details

Since data classes have a distinct runtime representation, HHVM will
need updating so `->` bytecodes work with this new datatype.

Data class splat updates may require a new bytecode entirely.

## Alternatives Considered

### Structs

Adding a simple 'bag of data' struct was also a [design we
explored](https://fb.intern.workplace.com/groups/468328013809203/permalink/541895673119103/).

```
record Person {
  string $name;
  bool $admin;
  ?string $email = null;
}
```

Since we want a nominally typed data model for performance, it would
not be feasible to codemod shapes code to use structs. We
would end up with a larger programming language with more features
that users must learn.

This also makes it difficult for us to explore adding methods in future.

### Alternative update syntax

```
$p2 = clone $p with name = "anonymous", email = "example@example.com";
```

Whilst `clone` is legal PHP/Hack syntax, it's not well known. The name
is also unfortunate, as it's a shallow copy. It's more verbose too.

```
$p2 = $p with name = "anonymous", email = "example@example.com";
```

This syntax would enable good code completion, but has little Hack
precedent. It also means data classes have more distinct syntax from
normal classes, making it hard for us to unify syntax in future.

### Alternative instantiation syntax

We plan to emit a different bytecode for data class
instantiation. This requires syntax that unambiguously represents a
record instantiation.

This bytecode exploits that data classes cannot be in a partially
constructed state, because they don't have constructors. HHVM can
treat it specially.

```
$p = Person(name = "anonymous");
$p2 = Person();
```

This is ambiguous with function calls when all the properties have
default values.

```
$p = new Person(name = "anonymous");
$p2 = new Person();
```

This is ambiguous with normal classes when all the properties have
default values.

### Reusing class declaration syntax

```
final data class User {
  public string $name;
  public bool $admin;
  public ?string $email = null;
}
```

This is significantly more verbose for limited benefit. We want this
to be a compelling alternative to shapes, which have a lightweight
syntax.

### Open Data Classes

```
// '...' allowing additional fields, like shapes.
data class OpenUser {
  string $name;
  bool $admin;
  ...
}
```

Similar to open shapes, this would additional fields to be added
dynamically.

This would require additional runtime work, and wouldn't enjoy the
performance benefits. We believe that an explicit `extra` property is
sufficient.

```
data class OpenUser {
  string $name;
  bool $admin;
  shape(..) $extra = shape();
}
```

## Open Questions

### Equality

We'd probably want structural equality for `===`.

```
Person[name = "x"] === Person[name = "x"]; // true
```


This is inconsistent with other forms of classes though.

It's also unclear what semantics `==` should have for data classes.

### Serialization

It's not clear how data classes should serialize and deserialize.

### Shape Interoperability

It's not clear what APIs we will offer for converting between data
classes and shapes. This is potentially a lossy transform, due to the
nominal typing of data classes and the observable ordering of shapes.

### Generics

We have avoided considering generics so far, to keep the typing rules
simple. We're hoping to ship v1 without generics.

This is definitely a feature we want to add after v1 is feature
complete and shipped.

### Nested Records

Shapes are copy-on-write, allowing updates of nested items.

```
$s['x']['y'] = 42;
```

There is no equivalent syntax for data classes, making this use case
more verbose.

### `$this` semantics

If we do add methods in the future, it is not clear how we handle
`$this`. Would methods be able to change `$this`? If not, what can
methods return?

### Field Ordering

HHVM has had performance wins by reordering fields in classes. We'd
like to retain that flexibility with data classes. If we really need
observable ordering for data classes, it should probably be opt-in.

## Prior Art

C# has [value
types](https://docs.microsoft.com/en-us/dotnet/standard/base-types/common-type-system?redirectedfrom=MSDN#structures)
which are introduced with the `struct` keyword. They do not support inheritance.

Scala has [case
classes](https://docs.scala-lang.org/tour/case-classes.html). They are
immutable classes with public properties, and also provide a `.copy`
method for creating modified copies. They use structural equality.

Kotlin has [data
classes](https://kotlinlang.org/docs/reference/data-classes.html). They
support inheritance and interfaces, and automatically provide equality
and copy methods.

