Start Date: April 19, 2022

Stage: DRAFT

## Summary

This feature, referred to as type _refinements_, adds the ability to specify additional constraints on the type being refined: each of its type constants or context constants can specify either an exact bound or loose (upper and/or lower) bounds. For example, provided the definition

```
interface Box { abstract const type T; const ctx C super [defaults]; }
```

the feature would allow writing types such as:

* `Box with { type T = int; ctx C super [globals]; }`
* `Box with { type T as arraykey }`


## Feature motivation

Type refinements is a feature that has a sound semantics and already exists in another widely used programming language; it would enhance Hack in 3 key areas:

* _expressiveness_ -- __more general form of where-constraints__; i.e., the existing where-constraints cannot span to a class level and cannot talk about the return type alone
* _soundness_ -- __more principled basis for type projections__, offering a limited form of dependent types needed to soundly model polymorphic contexts, for example.
* _convenience_ -- __reduces boilerplate Hack code__: no intermediate interfaces/classes need to be introduced to merely refine the constraints on type/context constants.


### More expressiveness

Considering the following definitions

```
interface ReadonlyBox {
  abstract const type T;
  public function get(): this::T;
}
interface TBox extends ReadonlyBox {
  public function set(this::T $val): void;
}
```

suppose we want to write a class that delegates to a _generic_ box that is provided via a constructor, where the class also provides a getter that returns an Readonly Box for wrapper (for safety reason):

```
class SafeBoxDelegator<Tref> {
  public __construct(private Box with { type T = Tref } $w) {}
  public function set(Tref $val): void { $this->b->set($val); }
  public function get(): ReadonlyBox with { type T = Tref } { … }
}
```

The above example is currently inexpressible in Hack for two reasons. First, the refinement in the constructor would require _class_-level where-clauses, which have been experimented with but are tricky because they do not have sound foundation. Second, an attempt to use where-clauses at the function level fails when there the type does not appear as part of “input” to the function:

```
public function get<TIB as ReadonlyBox, Tbad>(): TIB
        where Tbad = TIB::T { … }
```

The reason is that Hack would need to treat generic `Tbad` as an _existential_ type here (because it only holds for some instantiations), which is non-trivial during type inference. Other languages such as Scala have banned user-denotable existential types, and by further generalizing where clauses we would be dangerously close to the Scala-banned behavior.

### Replacement for unsound projections

One might attempt to work around the existing limitations of where clauses by changing the interface

of `SafeBoxDelegator` to accept the Box as a class generic, i.e.:

```
class SafeBoxDelegator<TBox as Box> {
  public __construct(private TBox $w) {}
  public function set(TBox::T $val): void { … }
            // error: ^^^^^^^
  public get(): ReadonlyBox with { type T = TBox::T } { … }
  …                             // error:   ^^^^^^^
}
```

However, this would result in errors because it is unsound to project off an abstract type (`TBox`, which is a subtype of `Box` and may not have its `T` defined yet).

Fortunately, the above can be soundly modeled if a refinement is added to the constraint of the class generic `TBox` as follows:

```
class SafeBoxDelegator<TinBox, TBox as Box with { type T = TinBox }> {
  public __construct(private TBox $w) {}
  public function set(TInBox $val): void { $this->b->set($val); }
  public function get(): ReadonlyBox with { type T = TInBox } { … }
}
```

Currently, Hack does allow a similar trick for expressing a limited form of dependent typing for cases where a where-clause does not span beyond function level: i.e., a function where the type of one parameter depends on the type of another, such as:

```
function set_box<T1, TBox as Box>(Tbox $b, T1 $v): void
    where T1 = TBox::T // OK
// different than: set_box(Box $b, Box::T $v): void  // ERROR
```

which is semantically different from the alternative signature in the comment. This is a common source of confusion, since there is _more than one way_ to write such functions using projections but _only one way is right_. Conversely, using the proposed feature results in a single and the correct solution,

```
function setBox<T1>(Box with { type T = T1; } $b, T1 $v): void
```

and is backed by a solid theoretical foundation in the [DOT calculus](http://lampwww.epfl.ch/~amin/dot/fool.pdf) [3,4] unlike the former approach that employs where-clauses and type projections.

As a final benefit, type refinements would serve as a more solid intermediate representation for dependent context constants in the typechecker. E.g.

```
interface BoxWithCtx extends Box { const ctx C; }
function useBox(BoxWithCtx $b)[$b::C]: void { … }
```

would be internally represented via refinement of context constant `C` (which desugars to a type in the typechecker)

```
function useBox<TC>(
    BoxWithCtx with { ctx C = [TC] } $b
)[Ctx]: void
```

as opposed to the current desugaring into where-clauses and projections of questionable soundness.

### Less definition boilerplate

Another use case is a bounded class generic whose usage throughout the class makes consistent assumptions on the bound. E.g., to parameterize with a subtype of `Box` whose type constant `T` is `num`eric, one could use the proposed feature,

```
class NumericComputation<TBox as Box with { type T as num; }> {
  public function run_iterative_until_convergence(
    vec<this::TBox> boxed_input_params,
    // input/output parameters (each assignment will be logged, which
    //                          requires `globals` context)
    Box with { type T = int; ctx C super [globals] } $lastIteration,
    this::TBox with { ctx C super [globals] } $lastIterationError,
  )[globals]: BoxWithCtx with { type T as vec<num>; ctx C = [] } { … }
}
```

Without the proposed solution, one would have to create “opaque” abstract class/interfaces -- introducing overhead -- and use concrete type, e.g.:

```
interface LoggedBox extends BoxWithCtx { abstract ctx C super [globals]; }
interface NumericBox extends Box { abstract const type T as num; }
interface LoggedNumericBox extends LoggedBox, NumericBox {}
interface LoggedIntBox extends LoggedBox { const type T = int; }
interface VecNumericBoxPure extends Box {
   abstract const type T as vec<num>;
   const ctx C = [];
}
```

However, this workaround not only degrades users’ experience but also _doesn’t scale_: it leads to an exponential number of “phantom” interfaces with combinations of constraints.


## User experience

### Syntax

 A type refinement would be written as `Classish with { … }` where `...` refines one or more type constants. An example is a function signature that returns a `Box` whose type constant `T` is fixed to `int` (the new syntax is overlined):

```
                           _____________________
function getIntBox(): (Box with { type T = int })
```

Each refinements inside `{`...`}` should be a valid type/context definition, possibly abstract, but must introduce either `=` or any combination of `as`/`super` bounds

#### EBNF notation

 Add rules:

```
TypeRefinement ::= TypeSpecifier, `with`, `{`, Refinement `}` ;
Refinement ::= (* empty *) | { RefineConst [`;`] } ;
RefineConst ::= RefineType | RefineCtx ;

RefineType ::= `type` QualifiedName ( `=` TypeSpecifier | RefineTypeBounds ) ;
RefineTypeBounds ::= RefineTypeBounds ( `as` | `super` ) TypeSpecifier
                                    | ( `as` | `super` ) TypeSpecifier ;

RefineCtx ::= `ctx` QualifiedName ( `=` CtxList | RefineCtxBounds ) ;
RefineTypeBounds ::= RefineTypeBounds ( `as` | `super` ) CtxList
                                    | ( `as` | `super` ) CtxList ;
```

Extend rule:

```
TypeSpecifier ::= (* new* ) TypeRefinement | (* old rules *) … ;
```

### Semantics

We should allow any type (hint) that represents a _class-like_ object to be optionally followed by `with { … }` syntax, with the following rules:

1. refinements cannot reference the `this` type, but paths/projections through it are allowed
2. projections off _abstract_ refined type alias/constant is allowed; e.g., `const type B = Box with { type T as int }` we can still refer to `B::T` but if Box wasn’t refined it would be unsound
3. projecting off an upper bound that is a refinement is still disallowed; e.g., in `const type B as Box with { type T as int }` we cannot refer to `B::T`

E.g., these should be allowed:

* `Box with { type T = int }`
* `BoxWithCtx with { ctx C super [defaults]; /* OK: T isn't refined */ }`
* ` `
    ```
    abstract class GoodBoxRefinements {
      const type TB = Box with { type T as int; };
      abstract function good_get(this::TB $box): this::TB::T; // OK (rule 2.)

    }
    ```

while these should be disallowed:

```
class BadBoxRefinementAndBadAbstractProjection {
  const type TBad = Box with { type T as this }; // ERROR (rule 1.)
  const type TB as Box with { type T = int };  // OK here
  public function bad_get(this::TB $box): this::TB::T; // ERROR (rule 3.)
}
```


## IDE experience:

### Auto-suggested type refinements

It would be useful if IDE could detect if a user attempts to use a certain type/context constants with implicit assumptions in mind that a quick-fix appears “Refine type member that belongs to …”, e.g.:

```
function reads_box_T_as_int(Box $b): int { return $b->get(); }
```

would ideally suggest “Refine type member in the type associated with parameter $b”, and accepting the quick-fix would refactor the above as:

```
function reads_box_T_as_int(Box with { type T as int; } $b): int {
  return $b->get();
}
```

### Good assistance if with keyword is forgotten

Users familiar with languages such as Scala that implement this feature with a similar syntax may accidentally write the following:

```
abstract function getIntBox(): Box { type T = int; }
```

which may misparse as a non-abstract function with a body containing a type definition:

```
abstract function getIntBox(): Box {
  type T = int; // hardly distinguishable from a body
}
```

If not the parser itself, the IDE should ideally suggest: “Did you mean Box with { … }?”


## Implementation details:

### Parser

After careful refactoring, most of the core parsing logic used by type and context constants is _reusable_. The only modified execution path is when a type specifier (hint) is parsed; the parser must check if `with` is the next token. The performance impact is therefore negligible, albeit it requires a lookahead of size 1 only when parsing XHP attributes as they can comprise multiple tokens (e.g., `attribute Type with-token2`).

### ASTs

TODO

### Typing

TODO

### HHVM

This feature is type-checker only because it does not provide a means of declaring anything new. Refinements merely give additional “hints” about the types associated with them (i.e., types before the `with` keyword), and are used by type inference only -- nothing can fail at run-time.

Therefore, HackC can simply ignore the entire refinement after a type. This is true even when a type refinement appear in an aliases or a type constant, e.g.:

```
type IntBox = Box with { type T = int }
```

will be bytecode-compiled identical to:

```
type IntBox = Box
```

which is sound for run-time because type aliases are not enforced, and neither are type constants.


## Design rationale and alternatives:

### Alternative syntax

One might argue why we need the `with` keyword before the braces unlike languages such as Scala where the syntax avoids the extra keyword. While this would be more compact, it would require tricks or very complicated lookahead and/or special-casing when parsing return types to discriminate between refinement and the function body (see section _IDE experience_ for an example).

There have also been wishes that the syntax was less verbose potential alternatives, e.g.:

```
Box[ T=int, C as[globals]super[zoned] ]
```

However, this would add significant parsing overhead as the parser would need to backtrack:

* parse a series of bounds with _context list_ if possible, otherwise
* parse a series of bounds with _type_.

If we want to prioritize reducing verbosity, there is hope that it can be done if we _unify representations_ of types and contexts -- at least for the purposes of type refinements, which is easier because this feature and syntax is invisible to HHVM as it doesn’t affect the runtime. Since context lists are already internally represented as types in the typechecker, we could express the above as follows:

```
Box[ T=int, C as CtxGlobal super CtxZoned ]
```


### Smarter where-clauses

TODO

### DOT-like object model

Generalizing Hack’s object model by basing it on the Dependend Object Types (DOT) calculus [3,4] is being actively considered as well, but it is a much larger undertaking. DOT-style path projections are _instance-based_ (`$obj.TypeOrCtx`) and would serve as a sound and more powerful substitute for type projections (`Type#TypeOrCtx`). Nonetheless, even languages that support instance-based projections (e.g., Scala) do offer refinements of types, attesting to the value added by this feature alone.


## Drawbacks:

This feature does not strip away or change existing functionality, so this section is largely inapplicable.

Nonetheless, it is a valid question to ask how Hack users are going to be educated and what assistance will be provided to help them choose between generics vs type constants with refinements.  For example, if the definition of `Box` above was written using generics, we would *not* need refinements but *would* end up hitting other limitations imposed by generics.  Until we better unify the notions of generics and type constants, we need to make the design intuitive in order to avoid confusing Hack users with seemingly interchangeable sets of features.


## Prior art:

### Type refinements in Scala

Scala 2 refers to the syntactic piece `TypeSpecifier with { … }` as _compound type_ (see [Sec 3.2.9](https://www.scala-lang.org/files/archive/spec/2.13/03-types.html#compound-types) of the official specification), whereas the `{ … }` is also called _refinement_. The `with` keyword is used for intersecting two types, which is superseded by the `&` operator that denotes _type intersection_ in Scala 3. Interestingly, Scala allows any sort of structural typing

* adding a new _value_ definition such as a method or field;
* adding a new _type_ definition that can contain equal or lower/upper bounds.

Notably, Scala allows introduction of _new_ types/values in refinements and thus offers a mechanism for _structural type_ refinements. E.g., we could declare an interface that accepts any subtype of Box that defines a new method:

```
def accepts_any_object_settable_to_int(
  box: AnyRef with { def set(v: Int): Unit }
)
```

Nonetheless, cyclic or self-referential accesses are prohibited in the refinement:

```
def all_args_are_invalid(
  box0: Box with { type T2 <: this.T }, // ERROR
  box1: Box with { def set(v: this.T): Unit }, // ERROR
                           // ----- self-ref disallowed
  box2: Box with { def set(v: box2.T): Unit }, // ERROR
)
```

even when they come from an outer object:

```
trait Outer { self => // refer to `Outer.this` as `self`
  type B = Box with { type T >: self.type }  // ERROR
}
```


### References:

[1] Scala 2.13 Specification, [Sec 3.2.9 (Compound Types)](https://www.scala-lang.org/files/archive/spec/2.13/03-types.html#compound-types)

[2] [EBNF notation](https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_form)

[3] paper: [Dependent Object Types](http://lampwww.epfl.ch/~amin/dot/fool.pdf)

[4] video: [Dependent Object Types](https://www.youtube.com/watch?v=b7AokpvwzgI)

[5] HIP: [context and coeffects](https://www.internalfb.com/code/fbsource/[a56b675db21fb8c075f735721a3147fe4e660822]/fbcode/hphp/hack/doc/HIPs/contexts_and_coeffects.md)


## Unresolved questions:

TODO


## Future possibilities

TODO
