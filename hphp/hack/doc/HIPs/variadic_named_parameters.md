**Status**: Draft, prototype implementation behind `variadic_named_parameters` and `named_variadic_type` typechecker options
**Audience**: Hack Improvement Proposal committee
**Author**: Andrew Kennedy

# HIP: Variadic Named Parameters

## Summary

The proposal consists of two features.

1. Support for a *named variadic* in a function type, e.g. `(function(int, named bool $flag, named string...):void)`
represents functions that can be applied to a single positional parameter of type `int`,
a named parameter `flag` of type `bool`, and any number of additional named parameters of type `string`.
For example, `foo(3, flag=false, other="A")` is a legal call to such a function,
and `foo(3, flag=false, other=3)` and `foo(3, flag=3, other="A")` are illegal calls.

2. Support for functions, methods and lambda definitions that accept any number of named parameters of a given type, absent the ability to make use of those parameters in the body.
For example, `(int $arg, named bool $flag, named string...) ==> $flag ? $arg : 0` is a function whose type is the one mentioned in (1) above.

## Motivation

Ultimately, we want to support fully generic variadics for named parameters, as with positional parameters, including the packaging of named parameters as `dict` (cf packaging of positional parameters in a `vec`), expression-level splatting of
a shape or dictionary into named parameters (cf `...$x` splatting for tuples into positional parameters), and type-level
splatting of shape types into named parameters in function types.

But that's a lot of design and implementation work. For the specific case of *typed mocking*, it's possible to cover most uses with features (1) and (2) described in the summary. For example, consider the `mockReturn` function, that currently does not support mocking of functions with named parameters.
```hack
public function mockReturn<Tr>(Tr $v): this
  where
    Tfun super (readonly function(mixed...)[]: Tr) {
      return $this->mockImplementation((mixed ...$_args)[]: Tr ==> $v);
  }
```
Using the two features proposed here, we can support named parameters, as follows:
```hack
public function mockReturn<Tr>(Tr $v): this
  where
    Tfun super (readonly function(mixed..., named mixed...)[]: Tr) {
      return $this->mockImplementation((mixed ...$_args, named mixed...)[]: Tr ==> $v);
  }
```
## Syntax

Function types are extended with the syntax `named t...` where `t` is the type of the named parameters. Some examples are listed below:
```hack
// Named variadic only
type T1 = (function(named int...):void);
// Positional variadic and named variadic
type T2 = (function(int, string..., named bool...):void);
// Explicit named parameter and variadic
type T3 = (function(named int $n, named string...):void);
```
The named variadic can appear anywhere in the parameter list, but there can be at most one named variadic.

Function definitions (top-level, method, and lambda) are extended with the same syntax, i.e. without a name for binding
the named variadic parameters. Some examples are listed below:
```hack
// Has type T1 from above
function foo1(named int...):void { }
// Has type T2 from above
function foo2(int $x, string... $args, named bool...):void { }
// Has type T3 from above
function foo3(named int $n, named string...):void { }
```

## Typing

Calls to functions whose types have variadic named parameters are type-checked as you'd expect: explicit and required named parameters must be bound to corresponding named arguments of the correct type, explicit but optional named parameters *may* be bound to an argument, and then any remaining named arguments must have the type specified by the named variadic. For example, using the examples from the previous section:
```hack
foo1(a=3); // legal
foo1(a=3, b=4); // legal
foo1(a=false); // illegal
foo2(3, x=false); // legal
foo2(3, "A", x=5); // illegal
foo3(n=2, a="A"); // legal
foo3(n="A", a="A"); // illegal
foo3(n=2, a=3); // illegal
```
Subtyping on function types is as follows. Given two function types F1 and F2, for F1 to be a subtype of F2, it is necessary that for every name `n`,

1. if F2 *requires* a named parameter `n` of type `t2` then F1 must also require a named parameter of type `t1` with `t2<:t1`, and

2. if F2 *allows* a named parameter `n` of type `t2` then F1  must also allow a named parameter of type `t1` with `t2<:t1`, and

3. if F2 *disallows* a named parameter `n` then F1 must also disallow a named parameter `n`.

Here,
* F *requires* a named parameter `n` of type `t` if it declares `named t $n`.
* F *allows* a named parameter `n` of type `t` if it declares `optional named t $n` or `named t $n` or `named t...`.
* F *disallows* a named parameter `n` if it does not declare any named parameter.

Another way of viewing subtyping is by elaboration to shapes: translate named parameters to a shape type, with required named parameters translated to required shape fields, optional named parameters translated to optional shape fields, and variadic named parameter translated to a typed open shape, e.g. `(function(named int $x, optional named string $y, named arraykey...):void)` becomes `(function(shape('x' => int, ?'y' => string, arraykey...)):void)`.

## Runtime

Prior to this change, if a function is called with a named argument whose name does not match any named parameter in the definition, an exception is raised. The new functionality is as follows: this exception is *not* raised if the function
declares a named variadic. The function itself does not have access to the parameter; it is simply dropped. There is no runtime enforcement of the type specified on the named variadic.

## Type structures

At present, there is no type structure support for named parameters in function types.

## Rollout

Initially, feature (1) is enabled in the typechecker only if the `.hhconfig` flag is set `named_variadic_type=true`.
Feature (2) is enabled only if the `.hhconfig` flag is set `variadic_named_parameters=true`.
Once the changes land in HHVM, the feature can be safely enabled in the type-checker.
