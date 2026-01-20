# Introduction

This section covers the different built-in types available in Hack.

## Primitive Types
Hack has the following primitive types:
[bool](/hack/built-in-types/bool),
[int](/hack/built-in-types/int),
[float](/hack/built-in-types/float),
[string](/hack/built-in-types/string), and
[null](/hack/built-in-types/null).

## Union Types
Hack supports union types, like:
* [num](/hack/built-in-types/num), where `int` and `float` are subtypes of `num`, and
* [arraykey](/hack/built-in-types/arraykey), where `int` and `string` are subtypes of `arraykey`.

## The Super Type
Hack's super type is [mixed](/hack/built-in-types/mixed), which represents any value. All other types are subtypes of `mixed`.

A few things to know when working with `mixed` as a type:
* The opposite of `mixed` is [nothing](/hack/built-in-types/nothing), a special type at the "bottom" of all other types.
* `mixed` is equivalent to `?nonnull`. [nonnull](/hack/built-in-types/nonnull) is a type that represents any value except `null`.

## Hack Arrays
There are three types of [Hack Arrays](/hack/arrays-and-collections/introduction). They are:
[vec](/hack/arrays-and-collections/vec-keyset-and-dict#vec),
[keyset](/hack/arrays-and-collections/vec-keyset-and-dict#keyset), and
[dict](/hack/arrays-and-collections/vec-keyset-and-dict#dict).

Though not built-in as types, other alternatives exist in [Hack Collections](/hack/arrays-and-collections/object-collections).

## Other Built-In Types
Hack has other built-in types too, like:
[enum](/hack/built-in-types/enum) (with [enum class](/hack/built-in-types/enum-class) and [enum class labels](/hack/built-in-types/enum-class-label)),
[shape](/hack/built-in-types/shape), and
[tuples](/hack/built-in-types/tuples).

## Function Return Types
Other types like [noreturn](/hack/built-in-types/noreturn) and [void](/hack/built-in-types/void) are only valid as function return types .

## Special Types
These last few types are special in their utility and/or versatility:
[classname](/hack/built-in-types/classname),
[dynamic](/hack/built-in-types/dynamic), and
[this](/hack/built-in-types/this).
