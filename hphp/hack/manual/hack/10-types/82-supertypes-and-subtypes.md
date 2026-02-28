# Supertypes & Subtypes

The set of built-in and user-defined types in Hack can be thought of as a type hierarchy of *supertypes* and *subtypes* in which a variable
of some type can hold the values of any of its subtypes. For example, `int` and `float` are subtypes of `num`.

A supertype can have one or more subtypes, and a subtype can have one or more supertypes. A supertype can be a subtype of some other supertype,
and a subtype can be a supertype of some other subtype.

The relationship between a supertype and any of its subtypes involves the notion of substitutability. Specifically, if *T2* is a subtype of *T1*,
program elements designed to operate on *T1* can also operate on *T2*.

For types in Hack, the following rules apply:
* The root of the type hierarchy is the type `mixed`; as such, every type is a subtype of that type.
* Any type is a subtype of itself.
* `int` and `float` are subtypes of `num`.
* `int` and `string` are subtypes of `arraykey`.
* For each type *T*, *T* is a subtype of the nullable type `?`*T*.
* For each type *T*, the null type is a subtype of all nullable types `?`*T*.
* `string` is a subtype of `Stringish`.
* The predefined types `vec`, `dict`, and `keyset` are subtypes of `Container`, `KeyedContainer`, `KeyedTraversable`, and `Traversable`.
* If *A* is an alias for a type *T* created using `type`, then *A* is a subtype of *T*, and *T* is a subtype of *A*.
* If *A* is an alias for a type *T* created using `newtype`, inside the file containing the `newtype` definition, A is a subtype of *T*, and *T*
is a subtype of *A*. Outside that file, *A* and *T* have no relationship, except that given `newtype A as C = T`, outside the file with the
`newtype` definition, *A* is a subtype of *C*.
* Any class, interface, or trait, having a public instance method `__toString` taking no arguments and returning string, is a subtype of `Stringish`.
* A class type is a subtype of all its direct and indirect base-class types.
* A class type is a subtype of all the interfaces it and its direct and indirect base-class types implement.
* An interface type is a subtype of all its direct and indirect base interfaces.
* A shape type *S2* whose field set is a superset of that in shape type *S1*, is a subtype of *S1*.
* Although [`noreturn`](/hack/built-in-types/noreturn) is not a type, per se, it is regarded as a subtype of all other types, and a supertype of none.
