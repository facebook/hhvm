We can create an alias name for a type, and it is common to do so for non-trivial tuple and shape types.  Once such a type alias has been defined,
that alias can be used in almost all contexts in which a type specifier is permitted.  Any given type can have multiple aliases, and a type alias
can itself have aliases.

## Quickstart
A type alias can be created in two ways: using `type` and `newtype`.

```Hack
type Complex = shape('real' => float, 'imag' => float);
newtype Point = (float, float);
```

A type alias can include [Generics](/hack/generics/introduction) as parameters.

## Using `type`
An alias created using `type` (such as `Complex` above) is a *transparent type alias*. For a given type, that type and all transparent aliases
to that type are all the same type and can be freely interchanged. `type` declarations are top-level declarations.

## Using `newtype`
An alias created using `newtype` (such as `Point` above) is an *opaque type alias*. In the absence of a type-constraint (see `Counter`
example below), each opaque alias type is distinct from its underlying type and from any other types aliasing it or its underlying type.
Only source code in the file that contains the definition of the opaque type alias is allowed access to the underlying implementation. As
such, opaque type aliasing is an abstraction mechanism. Consider the following file, which contains an opaque alias definition for a tuple
that mimics a point:

```Hack
newtype Point = (float, float);

function create_Point(float $x, float $y): Point {
  return tuple($x, $y);
}

function distance(Point $p1, Point $p2): float {
  $dx = $p1[0] - $p2[0];
  $dy = $p1[1] - $p2[1];
  return sqrt($dx*$dx + $dy*$dy);
}
```

## Choosing between `type` and `newtype`

Looking at the earlier example, being in the same source file as the alias definition, the functions `create_Point` and `distance` have direct access to the `float` fields in any `Point`'s tuple. However, other files will not have this same access.

Similarly, if a source file defines the following opaque alias...

```Hack
newtype Widget = int;
```

...any file that includes this file has no knowledge that a `Widget` is really an integer, so that the including file cannot perform any
integer-like operations on a `Widget`.

Consider a file that contains the following opaque type definition:

```Hack
newtype Counter = int;
```

Any file that includes this file has no knowledge that a Counter is really an integer, meaning the including file *cannot* perform any
integer-like operations on that type. This is a major limitation, as the supposedly well-chosen name for the abstract type, Counter,
suggests that its value could increase and/or decrease. We can fix this by adding a type constraint to the alias's definition, as follows:

```Hack
newtype Counter as int = int;
```

The presence of the type constraint `as int` allows the opaque type to be treated as if it had the type specified by that type constraint,
which removes some of the alias' opaqueness. Although the presence of a constraint allows the alias type to be converted implicitly to the
constraint type, no conversion is defined in the opposite direction. In this example, this means that a Counter may be implicitly converted
into an `int`, but not the other way around.

Consider the following:

```Hack no-extract
class C {
  const type T2 as arraykey = int;
  // ...
}
```

Here, we have a class-specific type constant that is an alias, which allows a value of type `T2` to be used in any context an `arraykey`
is expected. After all, any `int` value is also an `arraykey` value.
