Start Date: April 19, 2022

Stage: DRAFT

## Summary

This feature, referred to as _type refinements_, adds the ability to
specify structural constraints on a type. The type subject to the
constraints is said to be _refined_. In this HIP, type refinements
can only constrain type and context constants of a class/interface.
The constraints on such constants can be either _exact_ or _loose_.
An exact constraint fully specifies the constant while a loose
constraint is a combination of lower and/or upper bounds that may
match multiple concrete constants. In the future, type refinements
could be further extended to apply other constraints to other types.

We now give an example use of the feature. Assuming the following
interface definition
```
interface Box { abstract const type T; const ctx C super [defaults]; }
```

type refinements would allow writing types such as:

* `Box with { type T = int; ctx C super [globals]; }`
* `Box with { type T as arraykey }`


## Feature motivation

Type refinements is a feature that is present in the Scala programming
language where it was proved to be useful and safe. It would enhance
Hack in 3 key areas:

* _expressiveness_ -- __complementing where-constraints__; some
  useful constraints cannot be expressed with where constraints,
  but can be expressed with type refinements; e.g., constraints
  on type constants of class-level generic parameters, or constraints
  on an existentially quantified return type.
* _soundness_ -- __more principled basis for type accesses__;
  offering a limited form of dependent types that can help
  replace unsafe where constraints with type accesses and
  needed to soundly model polymorphic contexts, for example.
* _convenience_ -- __reduces boilerplate Hack code__; no intermediate
  interfaces/classes need to be introduced to merely refine the constraints
  on type/context constants.


### Increased expressiveness

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

suppose we want to write a delegator class that exposes the box's
content type via a generic parameter and also provides a getter
function returning a ReadonlyBox; using type refinements, we can
write

```
class SafeBoxDelegator<Tref> {
  public __construct(private Box with { type T = Tref } $w) {}
  public function set(Tref $val): void { $this->b->set($val); }
  public function get(): ReadonlyBox with { type T = Tref } { … }
}
```

The above example is currently inexpressible in Hack for two
reasons. First, the refinement in the constructor would require
class-level where clauses, which are not available today and would
in this case include one occurrence of poorly-supported type
accesses on generic variables. Second, a common attempt to use
where-clauses to type the getter function fails:

```
class SafeBoxDelegator<Tref> {
  ...
  public function get<TIB as ReadonlyBox>(): TIB
          where TIB::T = Tref { … }
```

This function prototype, while usable, is not practically
implementable. To implement it, the programmer has to write code
that returns an object that is in _all_ subtypes of ReadonlyBox
that satisfy the where constraint. This is not the intent of the
programmer, who likely meant to return _one_ subtype of ReadonlyBox
satisfying the constraint. In terms of type theory, `TIB` is
universally quantified when the intent was to existentially
quantify it.

### Replacement for unsound projections

One might attempt to work around the existing limitations of where
clauses by changing the interface of `SafeBoxDelegator` to accept
the Box as a class generic, i.e.:

```
class SafeBoxDelegator<TBox as Box> {
  public __construct(private TBox $w) {}
  public function set(TBox::T $val): void { … }
            // error: ^^^^^^^
  public get(): ReadonlyBox with { type T = TBox::T } { … }
  …                             // error:   ^^^^^^^
}
```

However, the definition above is rejected by Hack because type
accesses on generics are not permitted as type hints. More
generally, type accesses on generics are poorly supported by the
language; for example, the typechecker does not prevent instantiating
generics with class types that have abstract type constants, leading
to unsoundness.
With refinements, it is possible to constrain a class-level generic
parameter to define a type constant of interest. For example

```
class SafeBoxDelegator<TinBox, TBox as Box with { type T = TinBox }> {
  public __construct(private TBox $w) {}
  public function set(TInBox $val): void { $this->b->set($val); }
  public function get(): ReadonlyBox with { type T = TInBox } { … }
}
```

Currently, Hack does allow one way to express a limited form of dependent
typing (i.e., a function where the type of one parameter depends on the
type of another one)

```
function setBox<T1, TBox as Box>(Tbox $b, T1 $v): void
    where T1 = TBox::T
```

However, the boilerplate required in the accepted prototype is a common
source of confusion for users and, as explained above, is only very weakly
shielding them from unsound code.  Using type refinements we write

```
function setBox<T1>(Box with { type T = T1; } $b, T1 $v): void
```

And the function `setBox` can no longer recieve as first argument a
value of a type for which we do not precisely know what the associated
type constant is.

As a final benefit, type refinements would serve as a more solid
intermediate representation for dependent context constants in the
typechecker. E.g.

```
interface BoxWithCtx extends Box { const ctx C; }
function useBox(BoxWithCtx $b)[$b::C]: void { … }
```

would be internally represented via refinement of context constant
`C` (which is then further desugared into a type)

```
function useBox<TC>(
    BoxWithCtx with { ctx C = [TC] } $b
)[Ctx]: void
```

this new internal representation would replace the current desugaring
into where-clauses, generics, and projections of questionable soundness.

As a final remark about soundness, we will point out that unlike projections
off types, the type refinements feature has been well studied in the context
of Scala's core [DOT calculus](http://lampwww.epfl.ch/~amin/dot/fool.pdf)
[3,4].

### Less definition boilerplate

In the special case where a type refinement involves only concrete type
and context constants, it is possible to replace the refinement with
an additional subtyping constraint involving an interface that represents
the `with { … }` refinement component. Such interfaces may look like

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

However, this nominal encoding of refinements does not scale: it leads to
an exponential number of artificial interfaces with combinations of
constraints, significantly cluttering code and namespaces. In a sense,
we allow the use of some form of anonymous interfaces.

## Feature definition

We now expose in more details the specifics of type refinements. This
section aims to provide enough foundational information to successfully
complete an implementation of the feature.

### Syntax

A type refinement would be written as `Classish<…> with { … }` where
`{ … }` refines one or more type and context constants.  An example
is a function signature that returns a `Box` whose type constant `T`
is fixed to `int` (the new syntax is overlined):

```
                           _____________________
function getIntBox(): (Box with { type T = int })
```

Each refinement inside `{ … }` should be a valid type/context
constraint, it must contain either an exact `=` constraint or
an arbitrary combination of one or more `as`/`super` bounds.

#### EBNF notation

Add rules:

```
TypeRefinement   ::= TypeSpecifier `with` `{` Refinement [`;`] `}` ;
Refinement       ::= RefineType | RefineCtx | Refinement `;` Refinement  ;

RefineType       ::= `type` QualifiedName ( `=` TypeSpecifier | RefineTypeBounds ) ;
RefineTypeBounds ::= ( `as` | `super` ) TypeSpecifier RefineTypeBounds
                   | ( `as` | `super` ) TypeSpecifier ;

RefineCtx        ::= `ctx` QualifiedName ( `=` CtxList | RefineCtxBounds ) ;
RefineTypeBounds ::= ( `as` | `super` ) CtxList RefineTypeBounds
                   | ( `as` | `super` ) CtxList ;
```

Extend rule:

```
TypeSpecifier ::= (* new *) TypeRefinement | (* old syntax *) … ;
```

Note: Our draft implementation does not allow multiple sequenced
refinements and instead suggests to the user to merge them into
a single refinement.

### Well-formedness

In a refinement `Classish<…> with { … }`, the left-hand side classish
must be a fully applied interface, class, or type alias that
recursively expands to an interface or a class. This constraint means
that we currently do not support refining opaque types aliases (newtype),
the this type, or generics.

There is no constraint on the bounds of a refinement. In particular,
we may well have an unsatisfiable loose refinement. We will see in the
semantics section that the result of such a refinement is simply an
empty type equivalent to the bottom type nothing.

There is no requirement that the type/context constants in the
refinement are defined in the classish they refine. However, in this
case, a linter (TAST check) could warn the user and suggest constants
appearing in the classish with similar names.

### Typing rules

We only give typing rules for type constant refinements, context
constant refinements are handled very similarly.

First, we modify the type access expansion algorithm to have the
following new clauses:

```
taccess((C<…> with { type T = t'; … }), 'T') = t'
taccess((C<…> with { … }),              'T') = taccess(C<…>, 'T')
... (* old clauses *)
```

These additional rules guarantee that any existing typing logic
will successfully take exact refinements into account. Since type
accesses on abstract type constants are poorly handled in general,
we suggest to not implement any specific rules about loose
refinements in the type access expansion logic.

We define a new type member lookup unit. The goal is to eventually
have this unit take care of all the type access expansion logic.
Throughout the transition, we take care to keep this type member
lookup principled and stick to well-understood accesses.

```
lookupty(/*class*/ C, 'T') =
  if 'T' is Abstract(lowerBnd,upperBnd) in C:
    BOUNDED(lowerBnd,upperBnd)
  else if 'T' is Concrete(ty) in C:
    BOUNDED(ty,ty)
  else
    ERROR

lookupty((/*class*/ C with { R }), 'T') =
  if R['T'] is (super lowerBnd as upperBnd):
    merge(lookupty(C, 'T'), BOUNDED(lowerBnd,upperBnd))
  else if R['T'] is (= ty):
    merge(lookupty(C, 'T'), BOUNDED(ty,ty))
  else
    lookupty(C, 'T')

// Merge two lookup results
merge(ERROR,other)                       = other
merge(other,ERROR)                       = other
merge(BOUNDED(lo1,up1),BOUNDED(lo2,up2)) = BOUNDED((lo1|lo2),(up1&up2))
```

We also modify the subtyping algorithm so that it applies the
following rules. The rules are listed by order of precendence.

```
          t ⊢ R          t <: C<…>
    (<:1) ------------------------
            t <: C<…> with { R }


                C<…> <: t
    (<:2) ------------------------
           C<…> with { … } <: t
```

Thinking about inference, we note that it is important to apply the
rule (<:1) before applying any rule that goes looking up into the
bounds or extends/implements of the type on the left-hand side of a
subtype query. This ensures that we have the best information
possible when looking up type constants.

The first typing rule uses a new judgement that we define below.

```
            lookupty(C,'T') = BOUNDED(lo,up)
            lo' <: lo              up <: up'
            C ⊢ { R }
    (⊢l) --------------------------------------
           C ⊢ { type T super lo' as up'; R }


            lookupty(C,'T') = BOUNDED(lo,up)
            ty <: lo              up <: ty
            C ⊢ { R }
    (⊢e) --------------------------------------
                 C ⊢ { type T = ty; R }


    (⊢[]) -----------
            C ⊢ { }
```

### Semantics

In the spirit of the [Shack](https://github.com/facebookresearch/shack)
project that interprets Hack types as sets of values, we give a semantics
for type refinements by defining the interpretation of refinements

```
[[ C with { type T cstr } ]]Σ =
    [[ C ]]Σ ∩ { l ∈ loc |
                 l ↦ (tag, phi);
                 [[ class_def[tag].T ]]_ ⊢ [[ cstr ]]Σ }
```

That is, the constrained class type is all the objects in the class type
of C that, additionally, have a type member that satisfies the constraint
in the type. It is noteworthy that the semantics interpret the type
refinements as an intersection type.

The interpretation function [[ . ]]Σ is parameterized by Σ an environment
that contains the interpretation for ambient generic parameters. When
interpreting a type constant obtained from the class definition this
environment is irrelevant because type constants are currently required
to be independent from all generic parameters. The constraint in the
refinement, on the other hand, is interpreted using the ambient generic
parameters. One unknown is that the current typechecker implementation
allows using the 'this' type in type constant definition, and 'this' has
not been formalized in Shack yet.

Another shortcoming of these semantics is the lack of account for Hack's
parallel object model where classname values are used as singleton objects
for final abstract classes. This lack of precision in the semantics sheds
doubts about the interaction of type refinements with classname values
and their associated capabilities (call to static methods).

## IDE experience:

### Auto-suggested type refinements

It would be useful if IDE could detect if a user attempts to use a
certain type/context constants with implicit assumptions in mind that
a quick-fix appears “Refine type member that belongs to …”, e.g.:

```
function reads_box_T_as_int(Box $b): int { return $b->get(); }
```

would ideally suggest “Refine type member in the type associated with
parameter $b”, and accepting the quick-fix would refactor the above as:

```
function reads_box_T_as_int(Box with { type T as int; } $b): int {
  return $b->get();
}
```

### Good assistance if with keyword is forgotten

Users familiar with languages such as Scala that implement this feature
with a similar syntax may accidentally write the following:

```
abstract function getIntBox(): Box { type T = int; }
```

which may misparse as a non-abstract function with a body containing a
type definition:

```
abstract function getIntBox(): Box {
  type T = int; // hardly distinguishable from a body
}
```

If not the parser itself, the IDE should ideally suggest: “Did you
mean Box with { … }?”


## Implementation details:

### Parser

After careful refactoring, most of the core parsing logic used by type
and context constants is _reusable_. The only modified execution path is
when a type specifier (hint) is parsed; the parser must check if `with`
is the next token. The performance impact is therefore negligible, albeit
it requires a lookahead of size 1 only when parsing XHP attributes as
they can comprise multiple tokens (e.g., `attribute Type with-token2`).

### ASTs

TODO

### Typing

TODO

### HHVM

This feature is type-checker only because it does not provide a means of
declaring anything new. Refinements merely give additional “hints”
about the types associated with them (i.e., types before the `with`
keyword), and are used by type inference only -- nothing can fail at
run-time.

Therefore, HackC can simply ignore the entire refinement after a
type. This is true even when a type refinement appear in an aliases or
a type constant, e.g.:

```
type IntBox = Box with { type T = int }
```

will be bytecode-compiled identical to:

```
type IntBox = Box
```

which is sound for run-time because type aliases are not enforced,
and neither are type constants.


## Design rationale and alternatives:

### Alternative syntax

One might argue why we need the `with` keyword before the braces unlike
languages such as Scala where the syntax avoids the extra keyword. While
this would be more compact, it would require tricks or very complicated
lookahead and/or special-casing when parsing return types to discriminate
between refinement and the function body (see section _IDE experience_
for an example).

There have also been wishes that the syntax was less verbose potential
alternatives, e.g.:

```
Box[ T=int, C as[globals]super[zoned] ]
```

However, this would add significant parsing overhead as the parser would
need to backtrack:

* parse a series of bounds with _context list_ if possible, otherwise
* parse a series of bounds with _type_.

If we want to prioritize reducing verbosity, there is hope that it
can be done if we _unify representations_ of types and contexts --
at least for the purposes of type refinements, which is easier because
this feature and syntax is invisible to HHVM as it doesn’t affect the
runtime. Since context lists are already internally represented as types
in the typechecker, we could express the above as follows:

```
Box[ T=int, C as CtxGlobal super CtxZoned ]
```


### Smarter where-clauses

TODO

### DOT-like object model

Generalizing Hack’s object model by basing it on the Dependend Object
Types (DOT) calculus [3,4] is being actively considered as well, but it is
a much larger undertaking. DOT-style path projections are _instance-based_
(`$obj.TypeOrCtx`) and would serve as a sound and more powerful substitute
for type projections (`Type#TypeOrCtx`). Nonetheless, even languages that
support instance-based projections (e.g., Scala) do offer refinements
of types, attesting to the value added by this feature alone.


## Drawbacks

This feature does not strip away or change existing functionality,
so this section is largely inapplicable.

Nonetheless, it is a valid question to ask how Hack users are going to
be educated and what assistance will be provided to help them choose
between generics vs type constants with refinements.  For example,
if the definition of `Box` above was written using generics, we would
*not* need refinements but *would* end up hitting other limitations
imposed by generics.  Until we better unify the notions of generics and
type constants, we need to make the design intuitive in order to avoid
confusing Hack users with seemingly interchangeable sets of features.


## Prior art

### Type refinements in Scala

Scala 2 refers to the syntactic piece
`TypeSpecifier with { … }` as _compound type_ (see
[Sec 3.2.9](https://www.scala-lang.org/files/archive/spec/2.13/03-types.html#compound-types)
of the official specification), whereas the `{ … }` is also called
_refinement_. The `with` keyword is used for intersecting two types,
which is superseded by the `&` operator that denotes _type intersection_
in Scala 3. Interestingly, Scala allows any sort of structural typing

* adding a new _value_ definition such as a method or field;
* adding a new _type_ definition that can contain equal or lower/upper bounds.

Notably, Scala allows introduction of _new_ types/values in refinements
and thus offers a mechanism for _structural type_ refinements. E.g., we
could declare an interface that accepts any subtype of Box that defines
a new method:

```
def accepts_any_object_settable_to_int(
  box: AnyRef with { def set(v: Int): Unit }
)
```

Nonetheless, cyclic or self-referential accesses are prohibited in
the refinement:

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


### References

[1] Scala 2.13 Specification, [Sec 3.2.9 (Compound Types)](https://www.scala-lang.org/files/archive/spec/2.13/03-types.html#compound-types)

[2] [EBNF notation](https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_form)

[3] paper: [Dependent Object Types](http://lampwww.epfl.ch/~amin/dot/fool.pdf)

[4] video: [Dependent Object Types](https://www.youtube.com/watch?v=b7AokpvwzgI)

[5] HIP: [context and coeffects](https://www.internalfb.com/code/fbsource/[a56b675db21fb8c075f735721a3147fe4e660822]/fbcode/hphp/hack/doc/HIPs/contexts_and_coeffects.md)

[6] [Shack](https://github.com/facebookresearch/shack), formal semantics for Hack types

## Unresolved questions:

TODO


## Future possibilities

TODO
