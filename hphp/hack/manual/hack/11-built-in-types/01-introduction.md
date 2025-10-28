# Introduction

This section covers the different built-in types available in Hack.

## Primitive Types
Hack has the following primitive types:
[bool](/docs/hack/built-in-types/bool),
[int](/docs/hack/built-in-types/int),
[float](/docs/hack/built-in-types/float),
[string](/docs/hack/built-in-types/string), and
[null](/docs/hack/built-in-types/null).

## Union Types
Hack supports union types, like:
* [num](/docs/hack/built-in-types/num), where `int` and `float` are subtypes of `num`, and
* [arraykey](/docs/hack/built-in-types/arraykey), where `int` and `string` are subtypes of `arraykey`.

## The Super Type
Hack's super type is [mixed](/docs/hack/built-in-types/mixed), which represents any value. All other types are subtypes of `mixed`.

A few things to know when working with `mixed` as a type:
* The opposite of `mixed` is [nothing](/docs/hack/built-in-types/nothing), a special type at the "bottom" of all other types.
* `mixed` is equivalent to `?nonnull`. [nonnull](/docs/hack/built-in-types/nonnull) is a type that represents any value except `null`.

## Hack Arrays
There are three types of [Hack Arrays](/docs/hack/arrays-and-collections/introduction). They are:
[vec](/docs/hack/arrays-and-collections/vec-keyset-and-dict#vec),
[keyset](/docs/hack/arrays-and-collections/vec-keyset-and-dict#keyset), and
[dict](/docs/hack/arrays-and-collections/vec-keyset-and-dict#dict).

Though not built-in as types, other alternatives exist in [Hack Collections](/docs/hack/arrays-and-collections/object-collections).

## Other Built-In Types
Hack has other built-in types too, like:
[enum](/docs/hack/built-in-types/enum) (with [enum class](/docs/hack/built-in-types/enum-class) and [enum class labels](/docs/hack/built-in-types/enum-class-label)),
[shape](/docs/hack/built-in-types/shape), and
[tuples](/docs/hack/built-in-types/tuples).

## Function Return Types
Other types like [noreturn](/docs/hack/built-in-types/noreturn) and [void](/docs/hack/built-in-types/void) are only valid as function return types .

## Special Types
These last few types are special in their utility and/or versatility:
[classname](/docs/hack/built-in-types/classname),
[dynamic](/docs/hack/built-in-types/dynamic), and
[this](/docs/hack/built-in-types/this).
