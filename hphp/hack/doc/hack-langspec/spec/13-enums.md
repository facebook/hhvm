# Enums

## General

An *enumeration* consists of a set of zero or more named, constant values called *enumeration* constants having type `int` or `string`. Each distinct enumeration constitutes a different *enumerated* type. An instance of an enumerated type is called an `enum`. Each enumerated type has a corresponding type called the *underlying type* of that enumerated type. That underlying type is limited to `int` or `string`.

## Enum Declarations

**Syntax**
<pre>
<i>enum-declaration:</i>
  enum  <i>name</i>  <i>enum-base</i>  <i>type-constraint<sub>opt</sub></i>  {  <i>enumerator-list<sub>opt</sub></i>  }
<i>enum-base:</i>
  :  int
  :  string
<i>enumerator-list:</i>
  <i>enumerator</i> 
  <i>enumerator-list</i>  <i>enumerator</i> 
<i>enumerator:</i>
  <i>enumerator-constant</i>  =  <i>constant-expression</i> ;
<i>enumerator-constant:</i>
  <i>name</i>
</pre>

*name* is defined in [§§](09-lexical-structure.md#names); *type-constraint* is defined in [§§](05-types.md#general); and *constant-expression* is defined in [§§](10-expressions.md#constant-expressions).

**Constraints**

The underlying type designated by *enum-base* must be able to represent all the values of the enumerators defined in the *enumerator-list*.

If *type-constraint* is present, *enum-base* must be a subtype ([§§](05-types.md#supertypes-and-subtypes)) of *type-constraint*s *type-specifier*.

Each *constant-expression* must have type `int` or `string`.

The *names* of *enumeration-constants* declared in the same *enum-declaration* must be distinct.

A *constant-expression* must not refer directly or indirectly to the *name* of its own *enumeration-constant*.

**Semantics**

An *enum-declaration* defines an enumerated type by the name *name*. Enumerated type names are case-preserved ([§§](03-terms-and-definitions.md). 

The *name*s in an *enumerator-list* are declared as constants. Multiple *enumeration-constants* declared in the same *enum-declaration* may have the same *constant-expression* value. Different enumerated types can have *enumeration-constants* with the same *name*. When used, each *name* is qualified by prepending its parent enumerated type *name* and "`::`", in that order.

The type of each *enumeration-constant* is the type specified by *type-constraint*, if present; otherwise, the type of each *enumeration-constant* is the enumerated type in which it is defined.

An *enumeration-constant* can be used in any read-only context for an expression of its type.

A *constant-expression* can refer to the *name*s of other *enumeration-constants* in the same enumerated type. However, it must use their qualified names.

**Examples**

```Hack
enum BitFlags: int as int {
  F1 = 1;
  F2 = BitFlags::F1 << 1;
  F3 = BitFlags::F2 << 1;
}
// -----------------------------------------
enum ControlStatus: int {
  Stopped = 0;
  Stopping = 1;
  Starting = 2;
  Started = 3;
}
function processStatus(ControlStatus $cs): void {
  switch ($cs) {
  case ControlStatus::Started:
    …
    break;
  case ControlStatus::Starting:
    …
    break;
  case ControlStatus::Stopped:
    …
    break;
  case ControlStatus::Stopping:
    …
    break;
  }
}
```

This example defines `ControlStatus` to be an enumerated type with an underlying type of `int`. The enumerated type has the four named enumeration constants `Stopped`, `Stopping`, `Starting`, and `Started`. Each enumeration constant is initialized with the integer constant value, as shown. When called, the function `processStatus` is passed an enum having one of the four possible enumeration constant values.

```Hack
enum Permission: string {
  Read = 'R';
  Write = 'W';
  Execute = 'E';
  Delete = 'D';
}
enum Colors: int {
  Red = 3;
  White = 5;
  Blue = 10;
  Default = 3;  // duplicate value is okay
}
```

## The Predefined Enumerated Type Methods

All enumerated types behave as if they contained the following family of public, static methods.

```Hack
public static function assert(mixed $value): XXX
public static function assertAll(Traversable<mixed> $values): Container<XXX>
public static function coerce(mixed $value): ?XXX
public static function getNames(): array<XXX, string>
public static function getValues(): array<string, XXX>
public static function isValid(mixed $value): bool
```

where *XXX* is the enumeration type through which these methods are called.

Note: When called on an enumerated type that contains multiple enumeration constants having the same *constant-expression* value, `getNames` throws an exception of type `\HH\InvariantException`.

The methods are defined below:

Name  |  Purpose
----  |  -------
`assert`  |  Converts the given $value to the enum’s underlying type.
`assertAll`  |  Converts the given `Traversable` of values to the enum’s underlying type.
`coerce`  |  Converts the given `$value` to the enum’s underlying type.
`getNames`  |  Returns a map-like array of enumeration constant values and their names.
`getValues`  |  Returns a map-like array of enumeration constant names and their values.
`isValid`  |  Indicated whether the given `$value` is one of the values in the enum.
