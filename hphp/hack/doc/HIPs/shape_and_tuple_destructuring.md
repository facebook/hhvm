**Status**: Accepted, parsing and type-checking are implemented
**Audience**: Hack Improvement Proposal committee
**Author**: Max Heiber

# HIP: Shape and Tuple Destructuring

## Summary

```c#
shape('x' => $x, ?'y' => $y, 'z' => _) = $point;

// punning on lhs
shape($x, $y) = $point;

// ignore remaining fields
shape($x, ...) = $point;

// tuple destructuring: for compositionality
tuple($a, $b) = $tup;
tuple($a, optional $b) = $tup;

// ignore remaining fields
tuple($a, ...) = $tup;

// also allow punning on RHS for consistency and ergonomics
$point = shape($x, $y);

```

## Motivation

### Goal

- **Ergonomics**:
  - Helps avoid useless boilerplate
  - Users want the feature.
    - Some example requests: [1](https://fburl.com/workplace/v7tfdqaj) [2](https://fburl.com/workplace/11tsa7t5) [3](https://fburl.com/workplace/zsargvja) [4](https://fburl.com/workplace/2uv3sw3f) [5](https://fburl.com/workplace/hbhjst33) [6](https://fburl.com/workplace/8myxpd6d)
  - Don't have worse ergonomics than PHP (!!) which has [destructuring](https://www.php.net/manual/en/language.types.array.php#language.types.array.syntax.destructuring), including for shape-like arrays.

Adapted from real-world code (see [original](https://fburl.com/code/w3pbqgjg) and 1800 more in https://fburl.com/diff/dl2g4nen):

```
 $ctx = $config['request_context'];
 $service_name = $config['service_name'];
 $endpoint_override = $config['endpoint_override'];
 $endpoint_fallback = $config['endpoint_fallback'];
 $retry_policy = $config['retry_policy'];
 $retry_strategy = $retry_policy['strategy'];
 $log_level = $config['log_level'];
 $deploy_target = $config['deploy_target'];
 $version = $config['api_version'];
 $display_name = $config['display_name'];
 $health_check_types = $config['health_check_types'];
 $last_updated = $config['last_updated'];
```

After:

```
 shape(
   'request_context' => $ctx,
   $service_name,
   $endpoint_override,
   $endpoint_fallback,
   'retry_policy' => $retry_policy,
   $log_level,
   $deploy_target,
   'api_version' => $version,
   $display_name,
   $health_check_types,
   $last_updated,
 ) = $config;
```



We intend to meet the goal, with the following constraints:

- AI-friendliness (and friendly for humans too):
  - Avoid introducing things unlikely to be used frequently (harder to learn, harder to teach)
- safety:
  - make it harder for there to be unhandled shape fields, especially as code evolves
  - Punning eliminates trivial mappings of keys to variables, making it safer to skim both AI or human code in reviews
  - We are willing to trade off *perfect* safety in favor of ergonomics. (slogan: safety at the expense of ergonomics is often safety at the expense of safety). This is why we allow not mentioning optional keys in closed patterns against closed shapes. For example, `shape($a, $b) = $sh;` is allowed when `$sh` has type `shape(‘a’ => int, ‘b’ => int, ?’c’ => int)`.
- language evolvability:
  - Don't make it harder to add pattern matching later
  - Avoid complex feature interactions
  - Don't prematurely fill in the feature space. For example, we can take learnings from shape destructuring and apply them to future proposals for vec+tuple destructuring or for punning.

### Relevant Data

1. While `list` is extremely flexible in theory, devs **rarely use this flexibility**.

- Destructuring is primarily done on ***structured*** **data**:
  - The vast majority of `list` destructurings are done on tuples.
- Destructuring typically involves simple lvalues (only containing `list` and variables):
  - Nearly all lvalues in `list` are simple (var or list) in prod, non-generated code.
  - The bulk of complex lvalues come from a single pattern in tests where devs destructure into properties that are `__LateInit`.
- Function calls on the left-hand-side are essentially unused.

source: [analysis](https://fburl.com/hack_shape_destructuring_hip)

See also [Future Possibilities](#future-possibilities). Starting simple doesn’t mean we can’t complicate the feature with supporting KeyedContainer and destructuring into static properties etc. later if we need to, but I argue that the data supports the hypothesis that devs don’t need the messy version with weird semantics that is hard to optimize and reason about.

2. The feature is expected to have wide applicability

Accessing all required fields of a shape seems to be common in practice, so the feature will have wide applicability. According to a vibe analysis, a majority of shape accesses are in functions that access all fields. source: [analysis](https://fburl.com/hack_shape_destructuring_hip)–validation and examples in progress, take this point with a grain of salt as the analysis is complex.

## Solution

Our design is meant to gibe with the relevant data and the constraints above.
See examples in the [Solution](#solution) section below.

- **Destructuring is just pattern-matching for irrefutable patterns**:
  - No observable evaluation order:
    - by safety and AI-friendliness constraint: semantics should be predictable. And yet, PHP-style `list` evaluation order is right-to-left by list elements but left-to-right for array indices.
    - by language evolution constraint: observable evaluation order would make it harder to optimize and reason about pattern-matching, and we want destructuring to essentially be a special case of pattern matching
- **Only destructure structured data** for now:
  - gibes with how existing features are used in practice (see "relevant data" above)
  - language design principles, in particular compositionality
  - use cases for destructuring vecs are rare. If we do want to add vec destructuring, we will likely also need rest-on-the-LHS `vec[1, 2, ...$rest] = $v;`, best considered in a future proposal.
- **Predictable syntax and semantics**:
  - I suspect `KeyedContainer('a' ?? f() => f()[])`, while very general and flexible, would have discoverability and learnability issues for AIs and humans. This kind of syntax is unlikely to be represented in the training data due to differences from other languages. I suspect such a construction would be hard to learn in-context, given that the power features of `list` are so rarely used in practice.

### Syntax

```
  destructure_lvalue ::=
      | lvar
      | wildcard   // `_`, for ignoring fields/entries
      | destructure_shape
      | destructure_tuple

  destructure_shape ::=
      | shape(destructure_shape_entry, .., destructure_shape_entry)
      | shape(destructure_shape_entry, .., destructure_shape_entry, ...)
      | shape(...)

  destructure_shape_entry ::=
      | destructure_shape_field
      | destructure_optional_shape_field

  destructure_shape_field ::=
      | string_literal => destructure_lvalue
      | lvar

  destructure_optional_shape_field ::=
      | ?string_literal => destructure_lvalue
      | ?lvar

  destructure_tuple ::=
      | tuple(destructure_tuple_entry, .., destructure_tuple_entry)
      | tuple(destructure_tuple_entry, .., destructure_tuple_entry, ...)
      | tuple(...)

  destructure_tuple_entry ::=
      | destructure_lvalue
      | destructure_optional_tuple_field

  destructure_optional_tuple_field ::=
      | optional destructure_lvalue
```

Statement contexts where `destructure_lvalue` may appear:

```
destructure_lvalue = expr;
foreach (expr as destructure_lvalue) stmt
foreach (expr as destructure_lvalue => destructure_lvalue) stmt
let destructure_lvalue : type_hint = expr;
```

Some key choices:

- We allow punning. `shape($x)` is equivalent to `shape(‘x’ => $x)`
- We have both closed patterns (no …) and open patterns (contain …)
- We have both required matching `’key’ => value` and optional field matching `?’key’ => value`

Note: shape/tuple destructuring is

- not allowed in: function/method parameters nor catch bindings
- shape/tuple aren't allowed in `list` nor vice-versa in order to not expose evaluation order or allow arbitrary side effects, etc. We may find that there are no/few use cases of `list` remaining. Codemodding away most `list` and linting against its use may be in scope. Fully eradicating `list` is too much scope, imo.

### Naming errors

The following rules are for us to make evaluation order unobservable, for safety and simplicity:

- No variable may be assigned multiple times in a destructuring assignment.
- `list` is not allowed in shape/tuple destructuring patterns.
- shapes and tuples are not allowed in `list`.

Here's why:

- `list` evaluation order is observable, since side effects are allowed and exceptions can happen
- `list` evaluation order is arguably not predictable for users, since it's (the only?) part of Hack that is evaluated right-left (!).
- If `list` is allowed inside shapes, then shape evaluation order would leak, and we would have to choose between inheriting the bizarre behavior of `list` or introducing inconsistency in the language.

The naming errors will not be bypassable and will guarantee good behavior in prod. These restrictions will not be implemented in HHVM due to insufficient motivation for such enforcement, in my opinion. This means that devs will be able to observe assignment order in non-prod environments, if they run code that doesn't type-check. `tuple($a, $a) = tuple(1, 2); echo $a` reveals the (implementation-defined) evaluation order.

### Runtime “as if” semantics

We will compile to userland-style code in HackC, leveraging existing mechanisms in HackC and HHVM.

This semantics given here is “**as-if**”: our actual implementation may differ but should produce the same bindings of variables to values.

We sketch informally, using specific examples. I did take a stab at a greeked version of the semantics in the [Appendix: Correctness of Typing](#appendix-correctness-of-typing) section but find these examples easier to read.

**Closed shape pattern:**

```
// source
shape('x' => $x, 'y' => $y) = $point;

// desugared
$t = $point;
$x = $t['x'];
$y = $t['y'];
```

**Open shape pattern:**

```
// source
shape('x' => $x, ...) = $point;

// desugared
$t = $point;
$x = $t['x'];
```

The `...` produces no code. Only mentioned fields are accessed.

**Optional fields in patterns:**

```
// source:  T = shape('x' => int, ?'y' => int)
shape('x' => $x, ?'y' => $y) = $point;

// desugared
$t = $point;
$x = $t['x'];
$y = Shapes::idx($t, 'y');       // null if 'y' absent
```

Optional fields use `Shapes::idx`, which returns `null` for absent keys.

**Nested optional fields in patterns:**

```
shape('name' => $name,
      ?'address' => shape('city' => $city, ?'zip' => $zip)) = $person;

// desugared
$t1 = $person;
$name = $t1['name'];                           // EXACT: direct access
$t2 = Shapes::idx($t1, 'address', dict[]);     // dict[] sentinel if absent
$city = Shapes::idx($t2, 'city');              // NULLABLE: required, but ancestor optional
$zip = Shapes::idx($t2, 'zip');                // NULLABLE: optional
```

The `dict[]` sentinel is the key trick: when `'address'` is absent, `$t2` becomes `dict[]`, and all subsequent `Shapes::idx` calls return `null`. No if/else branching needed.

**Wildcard `_`:**

```
// source
shape('x' => $x, 'y' => _) = $point;

// desugared
$t = $point;
$x = $t['x'];
$t['y']; // accessed and ignored
```

Wildcard is not a variable and cannot be used in expression contexts.

Wildcard is not `$_`. `$_` is a variable–the type checker treats it differently from other variables in some cases, but not consistently, for example, users can pass around `$_`, as Kevin Viratyosin pointed out in [a paste](https://fburl.com/phabricator/hcf2pg5s).

Also note that we generate code for fields that are ignored with wildcard, as suggested by Kevin Viratyosin. A few reasons for this:
- This way the runtime behavior of destructuring is more likely to match that of pattern matching. Hack doesn’t have pattern matching yet, but if it does in the future, we are likely to check that wildcarded fields exist. Consider `let $x: mixed = EXPR; match $x with shape(‘x’ => _) ==> echo ‘has x’ | shape(‘y’ => _) ==> echo has ‘y`. In such cases, we are using the presence of a field to determine which branch to take.
- Accessing the field is a proof that the field exists. This may enable us to reduce like types `(~)` if we want to use the information.

**Basic tuple:**

```
// source
tuple($a, $b) = $pair;

// desugared
$t = $pair;
$a = $t[0];
$b = $t[1];
```

**Tuple with optional entry:**

```
// source
tuple($a, optional $b) = $pair;

// desugared
$t = $pair;
$a = $t[0];
$b = HH\idx($t, 1);                           // null if index 1 absent
```

Note: optional fields are rare in tuple types, so this `optional` syntax on LHS is expected to be rarely used. We provide it anyway to allow ease of refactoring or migration between shapes and tuples, and to make the language more predictable than if we had an asymmetry between shape and tuple destructuring. `optional` keyword is rare cross-linguistically but is consistently used in Hack for tuple types and tuple-like things (such as optional named parameters).

**Tuple with ellipsis (prefix match):**

```
// source
tuple($a, $b, ...) = $triple;

// desugared
$t = $triple;
$a = $t[0];
$b = $t[1];
// trailing elements ignored, no code generated for ...
```

### Runtime performance notes

This document specifies an “as if” semantics, but future work is to find a good implementation. We will try to avoid code bloat and leverage existing optimizations.

I’m considering taking the sketch above almost literally:
- The design is such that HackC will compile to bytecode without conditional branching: all branching will be downstream (HHBBC, interpreter, JIT).
- One tweak: instead of compiling to Shapes::idx, we’d likely compile to the Idx opcode, to skip the overhead of a function call. HackC already compiles away calls to `HH\Idx`.

These may be relevant code pointers that may help w.r.t. perf considerations (disclaimer: not an expert):

- We use `dict[]` a bunch for defaults, but I think it is cheap: [`staticEmptyDictArray()`](https://github.com/facebook/hhvm/blob/d911b10bc976d0a931/hphp/runtime/base/array-data-inl.h#L35)
- hackc [`emit_idx`](https://github.com/facebook/hhvm/blob/d911b10bc976d0a931/hphp/hack/src/hackc/emitter/emit_expression.rs#L1865) compiles to the `Idx` instruction.
- HHBBC [`Idx` handler](https://github.com/facebook/hhvm/blob/d911b10bc976d0a931/hphp/hhbbc/interp.cpp#L5825)
- JIT: [`emitBespokeShapesIdx`](https://github.com/facebook/hhvm/blob/d911b10bc976d0a931/hphp/runtime/vm/jit/irgen-bespoke.cpp#L1030) has handling for known layouts

### Typing

#### Type errors: overview

Restrict shape destructuring to known fields:

```c#
let $sh : shape(‘a’ => int, …) = ….
shape(‘a’, ‘b’, ...) = $sh; // Error, ‘b’ is not a known field of shape(‘a’ => int, …)
```

In closed shape destructuring patterns (no … in the pattern) all required fields must be listed:

```c#
let $sh : shape(‘a’ => int, ?'b' => int, 'c' => int) = ….
shape(‘a’, ?‘b’) = $sh; // Error, ‘c’ is missing in shape destructuring pattern. 'c' is required in shape('a' => int, ?'b' => int, 'c' => int, ...)
```

Ellipsis is required for destructuring an open shape:

```c#
let $sh : shape(‘a’ => int, …) = ….
shape(‘a’, ...) = $sh; // OK
shape(‘a’) = $sh; // Error, missing ellipsis: ... is required at the end of the field list when destructuring an open shape
```

Closed tuple destructuring patterns should respect arity

```c#
let $tup : tuple(int, int) = ….
tuple($x) = $sh; // Error: expected tuple pattern of arity 2 and got tuple pattern of arity 1
```

Ellipsis is required for destructuring open tuples:

```c#
let $tup : tuple(int, ...) = ….
tuple($x, ...) = $tup; // OK
```

#### Type warnings

For monotonicity of type checking, attempting to destructure an optional field as required will be a warning instead of an error. Example:

```c#
$sh = shape(‘x’ => 1, ‘y’ => 1);
shape(‘x’, ?‘y’) = $sh; // warn
```

#### Typing judgments

We use the following notation:

```
G                    typing environment, maps variables to types
G |-[m] p : T -| G'  pattern p at type T in mode m produces bindings G'
m                    mode: exact or nullable, used for dealing with patterns for optional keys
{ k : S | r }        shape type with required field k where k inhabits S
{ ?k : S | r }       shape type with optional field k where k inhabits S
T[i]                 type of i-th element in tuple type T (0-indexed)
?T                   nullable type, equivalent  to T | null, ??T = ?T
required(T)          set of required field names in shape type T
closed(T)            T is a closed shape (no ... in the type)
arity(T)             number of elements in tuple type T
keys(P)              set of field names in pattern P
G_0 + .. + G_n       disjoint union of typing environments (well-defined
                     because naming rules ensure no variable is bound twice)
..                   meta-level ellipsis uses two dots `..`, distinct from
                     the surface-syntax open marker ...
```

The judgment `G |-[m] p : T -| G'` reads: "in environment `G`, pattern `p` checked against type `T` in mode `m` produces extended environment `G'`."

The mode `m` is either **exact** or **nullable**:

- **exact** \-- the target value is known to have type `T`. Used when all ancestor fields are required.
- **nullable** \-- the target value might be `dict[]` or `null` instead of a value of type `T`, because some ancestor field was optional. Variables receive nullable types.

**DESTRUCTURE** \-- top-level entry point. We always start in exact mode:

```
  G |- e : T
  G |-[exact] p : T -| G'
----------------------------------------- DESTRUCTURE
G |- (p = e) -| G'
```

**VAR** \-- variable pattern (leaf). Mode determines nullability:

```
--------------------- VAR-EXACT
G |-[exact] $x : T -| G, $x : T


--------------------- VAR-NULLABLE
G |-[nullable] $x : T -| G, $x : ?T
```

In exact mode, the variable receives the exact type. In nullable mode, the variable receives the nullable type, because the value might be `null`.

**VAR-DISCARD** \-- discard variable `$_`. Ignores the value:

```
--------------------- VAR-DISCARD
G |-[_] $_ : T -| G, $_ : null
```

\> recall that we set $\_ to null to avoid having evaluation order be observable.

When the variable is `$_`, the mode is irrelevant. The type is `null` because the runtime always assigns `null` without accessing the value. `$_` may appear multiple times in a pattern (it is the only variable exempt from the uniqueness requirement).

**FIELD-REQ** \-- required shape field. Passes mode through unchanged:

```
  T = { k : S | r }
  G |-[m] p : S -| G'
--------------------- FIELD-REQ
G |-[m] ('k' => p) : T -| G'
```

The inner pattern is typed at the field's type `S`. The mode `m` is inherited from the enclosing context \-- this is how nullable mode propagates through required fields under an optional ancestor.

**FIELD-OPT** \-- optional shape field. Switches to nullable mode:

```
  T = { ?k : S | r }
  G |-[nullable] p : S -| G'
--------------------- FIELD-OPT
G |-[_] (?'k' => p) : T -| G'
```

The inner pattern is typed at `S` (the field's declared type, not `?S`) in **nullable** mode. The input mode is irrelevant (written as `_`).

The nullability is NOT applied to the type `S` here \-- it is pushed down to the leaves via VAR-NULLABLE. This is essential: if we typed the inner pattern at `?S`, then SHAPE-EXACT (below) could not fire, because `?S` is a nullable type, not a shape type. By keeping the type as `S` and using nullable mode instead, compound patterns (shapes, tuples) can recurse normally while leaves correctly receive nullable types.

**SHAPE-EXACT** \-- closed shape pattern with all required fields present:

```
  closed(T)
  required(T) \subseteq keys(P)
  G |-[m] f_i : T -| G_i    for each field f_i in P
----------------------------------------- SHAPE-EXACT
G |-[m] shape(f_0, .., f_n) : T -| G_0 + .. + G_n
```

**SHAPE-OPEN** \-- open shape pattern (with `...`), ignoring unmentioned fields:

```
  G |-[m] f_j : T -| G_j    for each field f_j in P
----------------------------------------- SHAPE-OPEN
G |-[m] shape(f_0, .., f_n, ...) : T -| G_0 + .. + G_n
```

**TUPLE-EXACT** \-- tuple pattern matching all positions:

```
  arity(T) = n
  G |-[m] p_i : T[i] -| G_i    for each i in 0..n-1
----------------------------------------- TUPLE-EXACT
G |-[m] tuple(p_0, .., p_{n-1}) : T -| G_0 + .. + G_{n-1}
```

**TUPLE-OPEN** \-- tuple pattern matching a prefix (with `...`):

```
  k <= arity(T)
  G |-[m] p_i : T[i] -| G_i    for each i in 0..k-1
----------------------------------------- TUPLE-OPEN
G |-[m] tuple(p_0, .., p_{k-1}, ...) : T -| G_0 + .. + G_{k-1}
```

**TUPLE-ENTRY-OPT** \-- optional tuple entry (`optional` keyword). Switches to nullable mode, analogous to FIELD-OPT for shapes:

```
  G |-[nullable] p : T[i] -| G'
--------------------- TUPLE-ENTRY-OPT
G |-[_] (optional p at position i) : T -| G'
```

#### Typing examples

##### Example 1: Simple (all required, closed)

```
let $point: shape('x' => int, 'y' => int) = EXPR;
shape('x' => $x, 'y' => $y) = $point;
```

Derivation (read bottom-to-top; premises above the line, rule name to the right):

```
D_1:
                            -------------------------------- VAR-EXACT
  T = { 'x' : int | r }    G |-[exact] $x : int -| {$x:int}
  ----------------------------------------------------------- FIELD-REQ
  G |-[exact] ('x' => $x) : T -| {$x : int}

D_2:
                            -------------------------------- VAR-EXACT
  T = { 'y' : int | r }    G |-[exact] $y : int -| {$y:int}
  ----------------------------------------------------------- FIELD-REQ
  G |-[exact] ('y' => $y) : T -| {$y : int}

  closed(T)    required(T) \subseteq keys(P)    D_1    D_2
  ---------------------------------------------------------- SHAPE-EXACT
  G |-[exact] shape('x'=>$x, 'y'=>$y) : T -| {$x:int, $y:int}

  G |- $point : T    (above)
  ---------------------------------------------------------- DESTRUCTURE
  G |- (shape('x'=>$x, 'y'=>$y) = $point) -| {$x:int, $y:int}
```

Both variables receive exact (non-nullable) types because all fields are required and the pattern is closed.

##### Example 2: Open pattern on a closed shape

```
let $point: shape('x' => int, 'y' => int) = EXPR;
shape('x' => $x, ...) = $point;
```

Derivation:

```
D_1:
                            -------------------------------- VAR-EXACT
  T = { 'x' : int | r }    G |-[exact] $x : int -| {$x:int}
  ----------------------------------------------------------- FIELD-REQ
  G |-[exact] ('x' => $x) : T -| {$x : int}

  D_1
  ----------------------------------------------------------- SHAPE-OPEN
  G |-[exact] shape('x'=>$x, ...) : T -| {$x : int}

  G |- $point : T    (above)
  ----------------------------------------------------------- DESTRUCTURE
  G |- (shape('x'=>$x, ...) = $point) -| {$x : int}
```

##### Example 3: Nested optional with open inner shape

```
let $person: shape(
  'name' => string,
  ?'address' => shape('city' => string, 'state' => string, ?'zip' => string),
) = EXPR;
shape('name' => $name,
      ?'address' => shape('city' => $city, ...)) = $person;
```

Derivation (abbreviations: \[e\] \= exact, \[n\] \= nullable):

```
D_1 (name field):
                                ----------------------------------- VAR-EXACT
  T = { 'name' : string | r }  G |-[e] $name : string -| {$name:string}
  ----------------------------------------------------------------------- FIELD-REQ
  G |-[e] ('name' => $name) : T -| {$name : string}

D_2 (city field, nullable mode -- ancestor was optional):
                                ------------------------------------ VAR-NULLABLE
  S = { 'city' : string | r' } G |-[n] $city : string -| {$city:?string}
  ------------------------------------------------------------------------ FIELD-REQ
  G |-[n] ('city' => $city) : S -| {$city : ?string}

D_3 (inner shape, open pattern, nullable mode):
  continued in D_2
  --------------------------------------------------------- SHAPE-OPEN
  G |-[n] shape('city' => $city, ...) : S -| {$city : ?string}

D_4 (optional address field -- mode switches to nullable here):
  T = { ?'address' : S | r }   continued in D_3
  --------------------------------------------------------- FIELD-OPT
  G |-[_] (?'address' => shape('city'=>$city, ...)) : T -| {$city : ?string}

  closed(T)    required(T) \subseteq keys(P)    continued in D_1, D_4
  ---------------------------------------------------------- SHAPE-EXACT
  G |-[e] shape('name'=>$name, ?'address'=>shape(..)) : T
      -| {$name:string, $city:?string}

  G |- $person : T    (see above)
  ---------------------------------------------------------- DESTRUCTURE
  G |- (.. = $person) -| {$name : string, $city : ?string}
```

#### Correctness of type checking

* For any given variable used in a tuple or shape pattern in a destructuring assignment, our runtime semantics say how to evaluate and our typing judgments say how to type. The value of each variable assigned in a destructuring pattern should inhabit the set corresponding to the type the judgments give the type variable.
* Evaluation order should not be observable. This is also covered in the formalization linked above, but hopefully is also an obvious property given the design.

[See formalization](https://fburl.com/workplace/jsgp3bjp), with the caveat that in the formalization we pretend `null` is the sentinel value for optional fields rather than `dict[]`, which shouldn’t make a difference (`idx(null, ‘some_key’) === idx(dict[], ‘some_key’) === null`).

### Punning in shape expressions

We introduce punning in shape construction:

```
$point = shape($x, $y);
```

Motivation 1: Users will likely expect punning to work on the right hand side, given the symmetries between construction and destruction.

Construction with punning:

```
$point = shape($x, $y);
```

Destruction with punning:

```
$shape('x', 'y') = $point;
```

Motivation 2: users are likely to love it

Motivation 3: Punning in shape expressions is familiar from other languages, including Flow.

The runtime and typing rules follow from:

```
shape($x, $y);
```

desugars to:

```
shape('x' => $x, 'y' => $y);
```

Punned and non-punned fields can be freely mixed: `shape($x, 'y' => $y)` also desugars to `shape('x' => $x', 'y' => $y)`.

Punning is not allowed for `$_` – doing so could be misleading, since we assign `$_` to `null`. There are only [a few](https://fburl.com/hack_shape_destructuring_hip) uses of a ‘\_’ shape key in practice, so the restriction won’t arise often.

Punning similar to the feature proposed here is also found in JS/TS/Flow, OCaml, Haskell (with an [extension](https://ghc.gitlab.haskell.org/ghc/doc/users_guide/exts/record_puns.html)):

## Drawbacks

We are adding something to the language, which humans and agents will have to learn how to read, write, and modify. This does have a cost.

That said, the features are similar enough to those of languages like Flow and TypeScript that it doesn’t seem to be hard for LLMs like Claude to understand it, even though Claude was a little doubtful that the code was valid. Transcripts (grain of salt, small sample size): [1](https://claude.ai/share/0122e9fc-c97d-4ac8-82d5-e27059bed35e), [2](https://claude.ai/share/984881d6-fecb-4460-b852-3be9591508ca), [3](https://claude.ai/share/7fb7edf6-400d-4475-a128-ba154b73add4), [4](https://claude.ai/chat/d761c1c4-6fa0-4a7c-92b5-0cb41967434c), [5](https://claude.ai/share/f864e7ac-f49d-4a74-95e3-2ca82d52c48e), [6](https://claude.ai/share/ced5b6ca-210d-413d-a614-a9fbc57d3252), 82-93% of 50 subagents in a transcript.

## Future Possibilities

### Future work: expressive defaults

> Note: maybe even this could be optimized sometimes by HHVM: `shape(.....) = merge($defaults, $sh)`

The current proposal does not distinguish between missing fields and fields whose value is `null`. The distinction does not seem to be generally relevant for destruction, and the information can be recovered via `Shapes::keyExists`.

That said, sometimes `null` is not the best default and devs want to default to things like `vec[]`. However, type-directed defaulting would require type-directed bytecode.

A previous proposal considered `shape('x' ?? 2 => $x)`, but there were many open questions about syntax and semantics.

[As Kendall suggested](https://fburl.com/workplace/2w0s2x1b), by supporting shape merge on the right-hand-side, we can avoid the contentious syntax and unknown semantics of defaults in destructuring.

A future HIP (in progress, contact Michael Thomas) may address shape merge and enable more expressive defaults via an expression on the RHS.

### Potential future work: `as` in patterns

It’s common in pattern matching to allow binding of nested patterns. `shape(‘x’ => (shape(‘y’ => $y, …) as $inner))` , which would bind `$inner` to the result of the inner shape pattern.

We defer “as patterns” to potential future work rather than planning it now, because:
- `as` patterns typically don’t even save devs much keyboarding. The “long version” in this example is only 5 more characters and (arguably) easier to read:
`shape(‘x’ => $inner);`
`shape(‘y’ => $y, …) = $inner;`
- Given nested destructuring with `list` is uncommon today, we expect nesting to be uncommon for shape and tuple destructuring. A special language feature for rare cases of uses of something already rare may impair learnability of `as` in destructuring.
- Nothing in the current proposal precludes adding `as` patterns later. Or adding later something of equivalent expressiveness– Andrew Kennedy pointed out that “and patterns” are at least as expressive.

### Potential future work: weird semantics

We *could* allow things that lead to observable evaluation order, having arbitrary side effects, etc. The most compelling such feature, imo, is to allow property accesses:

`shape(‘x’ => $this->x, ‘y’ => $this->y) = $point;`

Some reasons not to allow property access in destructuring patterns are:
- Destructuring into properties is currently allowed via `list` (which works on tuple, vec, containers) but the feature is **essentially unused** in prod code (see Relevant Data above).
- Allowing complex lvalues would make it harder to maintain the language and to enable optimized pattern matching (or make pattern matching and destructuring very different), etc.
- Aren’t in any other language except PHP, afaict, and are not theoretically well-founded.

I think it’s safer to do the simpler thing first and then do the weird thing that only PHP does only if we must.

### Potential future work: variadics

The current proposal does not support the following operations:

```
shape('x' => 'y', ...$rest) = $open_shape;
tuple('x', 'y', ...$rest) = $tuple
```

We could consider one or both of these in a future proposal, though I worry a bit about perf of shape variadics.

### Potential future work: pattern matching

The hope is that if/when we introduce patterns for shapes and tuples, the syntax and semantics will be learnable and predictable for agents+users because destructuring just *is* irrefutable pattern matching. Re the implementation: we haven't closed off opportunities like easily-optimizable and theoretically well-understood pattern matching.

### Potential future work: deprecate `list`

I suspect that, long-term, large codebases would be in a better place with shape/tuple and no `list`. After launch, I would like to do medium-priority work to gradually codemod `list` to `tuple` in non-generated code where doing so can’t change runtime behavior. Then, ~six months later, evaluate feasibility and priority of more aggressive `list` deprecation or removal. This gap gives us time to incorporate user feedback and our experience from the codemodding.

### Potential future work: rationalize `$_` and `_` (dollar underscore and wildcard)

Status quo: `$_` really wants to be a “discard/throwaway” variable and is often used that way, but the type checker only sometimes special-cases it, and often treats `$_` as if it were any other variable ([example](https://fburl.com/phabricator/hcf2pg5s)).

State after we implement this HIP: users will need to know to use `_` to discard things in some places and `$_` in other places. That sucks.

Desired state: use `_` consistently for wildcard. Allow it most places a variable is used. Codemod away `$_`.

## Prior Art

- [Previous proposal by Scott Owens](https://fburl.com/diff/njw23trz).
- [Destructuring in PHP](https://www.php.net/manual/en/language.types.array.php#language.types.array.syntax.destructuring)
- Punning in JS/TS/Flow, Ocaml, and Haskell ([extension](https://ghc.gitlab.haskell.org/ghc/doc/users_guide/exts/record_puns.html))
- Destructuring in JS and pattern-matching in `let` in Rust and OCaml
- Python pattern matching as [an example of how to violate our language design principles](https://www.hillelwayne.com/post/python-abc/)
- For enabling pattern matching on arbitrary userland types with very flexible semantics (not necessarily a good idea):
  - Kotlin’s [destructuring declarations](https://kotlinlang.org/docs/destructuring-declarations.html)
  - [Python class patterns](https://peps.python.org/pep-0634/#class-patterns)
  - Scala [extractor objects](https://docs.scala-lang.org/tour/extractor-objects.html)


## Appendix: Correctness of Typing

Disclaimers:
- Canonical source runtime semantics of shape and tuple destructuring is given above in the HIP proper. This document is designed to give some flavor of what it means for typing for destructuring to be “correct”.
- An oversight: the following does *not* say how destructuring interacts with union types nor dynamic, though the behavior is strongly suggested by this HIP.

### Runtime

We described the operational semantics by example in the [Runtime "as if" semantics](#runtime-as-if-semantics) section above. I’ll take a stab at something more symbolic here, though it’s challenging to read and write.

Note that this is an “as if” semantics, and may not represent the actual implementation–the important thing is that the real implementation should produce the same bindings of variables to values as this as-if semantics.

This is a supplement to the main HIP above.

### Notation

The runtime desugaring is defined by three mutually recursive functions.

```
| Function | Signature | Purpose |
|---|---|---|
| `desugar` | `(||pat||, expr) -> S` | Top-level entry point. Binds `expr` to a fresh temporary and matches in EXACT mode. |
| `match` | `(||pat||, t, mode) -> S` | Matches pattern against variable `t` in the given mode. |
| `entry` | `(||field||, t, mode) -> S` | Processes a single shape field against variable `t`. |
```

Arguments:

- **`||pat||`** \-- a destructuring pattern (see Syntax section), written in denotation bars `|| ||` to mark it as surface-level syntax being interpreted by the metatheory.
- **`||field||`** \-- a shape entry: `||'k' => pat||` or `||?'k' => pat||`.
- **`expr`** \-- an arbitrary Hack expression.
- **`t`** \-- a Hack variable (compiler-generated temporary or source variable).
- **`mode`** \-- EXACT or NULLABLE:
  - **EXACT** \-- required fields use direct access `t['k']`.
  - **NULLABLE** \-- required fields use `Shapes::idx(t, 'k')`, because `t` might be `dict[]` or `null`.
- **`S`** \-- an ordered sequence of Hack statements. `\cup` denotes concatenation.
- **"fresh temp"** \-- a compiler-generated variable (e.g., `$__destructure_0`).
- **`_`** \-- wildcard: any value; the parameter is unused.
- **`~>`** \-- "desugars to": used in derivation annotations to show how a metatheory function call produces generated Hack code.

**`Shapes::idx` semantics** (for reference):

- `Shapes::idx(t, 'k')` \-- returns `t['k']` if key `'k'` is present, else `null`.
- `Shapes::idx(t, 'k', default)` \-- returns `t['k']` if key `'k'` is present, else `default`.
- When `t` is `null`: returns the default (`null` for 2-arg, the explicit default for 3-arg).

**`HH\idx` semantics** (for tuples):

- `HH\idx(t, i)` \-- returns `t[i]` if index `i` is present, else `null`.

### Rules

**Top-level:**

```
desugar(||pat||, e) =
  let t = fresh temp
  { t = e; } \cup match(||pat||, t, EXACT)
```

**M-Var** \-- variable pattern (leaf):

```
match(||$x||, t, _) = { $x = t; }
```

**M-Discard** \-- discard variable `$_`. Skips the value:

```
match(||$_||, t, _) = { $_ = null; }
```

The runtime does not read `t`; it unconditionally assigns `null`.

**M-Shape** \-- shape pattern:

```
match(||shape(E_1, .., E_n)||, t, mode) =
  entry(||E_1||, t, mode) \cup .. \cup entry(||E_n||, t, mode)
```

**E-Req-Var** \-- required field, variable (`$x != $_`):

```
entry(||'k' => $x||, t, EXACT)    = { $x = t['k']; }
entry(||'k' => $x||, t, NULLABLE) = { $x = Shapes::idx(t, 'k'); }
```

When the variable is `$_`, the field access is skipped:

```
entry(||'k' => $_||, t, _) = { $_ = null; }
```

**E-Req-Nested** \-- required field, nested pattern:

```
entry(||'k' => pat||, t, EXACT) =
  let t' = fresh temp
  { t' = t['k']; } \cup match(||pat||, t', EXACT)

entry(||'k' => pat||, t, NULLABLE) =
  let t' = fresh temp
  { t' = Shapes::idx(t, 'k', dict[]); } \cup match(||pat||, t', NULLABLE)
```

**E-Opt-Var** \-- optional field, variable (`$x != $_`):

```
entry(||?'k' => $x||, t, _) = { $x = Shapes::idx(t, 'k'); }
```

When the variable is `$_`, the field access is skipped:

```
entry(||?'k' => $_||, t, _) = { $_ = null; }
```

**E-Opt-Nested** \-- optional field, nested pattern:

```
entry(||?'k' => pat||, t, _) =
  let t' = fresh temp
  { t' = Shapes::idx(t, 'k', dict[]); } \cup match(||pat||, t', NULLABLE)
```

`Shapes::idx(t, 'k', dict[])` returns: the inner shape when `'k'` is present and non-null; `null` when `'k'` is present but null (possible with nullable field types); `dict[]` when `'k'` is absent. In the latter two cases, subsequent `Shapes::idx` calls on `null` or `dict[]` return their defaults, so all descendant variables become `null`.

**M-Tuple** \-- tuple pattern:

```
match(||tuple(p_0, .., p_{k-1})||, t, mode) =
  { $x_0 = t[0]; } \cup match(||p_0||, $x_0, mode)
  \cup .. \cup
  { $x_{k-1} = t[k-1]; } \cup match(||p_{k-1}||, $x_{k-1}, mode)
```

Each element is accessed by index. When `p_i` is a variable, `match` reduces to M-Var and the intermediate temp is unnecessary (an implementation may optimize this away).

**T-Entry-Opt** \-- optional tuple entry (`optional` keyword):

```
match(||optional p at position i||, t, mode) =
  let t' = fresh temp
  { t' = HH\idx(t, i); } \cup match(||p||, t', NULLABLE)
```

`HH\idx(t, i)` returns `t[i]` if index `i` is present, else `null`. This is analogous to `Shapes::idx` for shapes.

**Ellipsis (`...`) is a runtime no-op.** The `...` marker in both shape and tuple patterns affects only which entries appear in the pattern. The runtime rules iterate over the entries present in the pattern and ignore everything else. There is no generated code for `...` itself.

### Runtime example

For more examples, see the [Runtime "as if" semantics](#runtime-as-if-semantics) section above.
The result of desugaring (right-hand-side of \~\>) is a sequence of assignment statements

```
desugar(||shape('name' => $name,
               ?'address' => shape('city' => $city, ...))||,
        $person)

  = { $t1 = $person; }
    \cup entry(||'name' => $name||, $t1, EXACT)
         ~> { $name = $t1['name']; }
    \cup entry(||?'address' => shape('city' => $city, ...)||, $t1, EXACT)
         = { $t2 = Shapes::idx($t1, 'address', dict[]); }   -- mode switches to NULLABLE
           \cup entry(||'city' => $city||, $t2, NULLABLE)
                ~> { $city = Shapes::idx($t2, 'city'); }
```

**Desugared Hack:**

```
$t1 = $person;
$name = $t1['name'];                              // direct access (EXACT)
$t2 = Shapes::idx($t1, 'address', dict[]);        // idx + dict[] fallback
$city = Shapes::idx($t2, 'city');                  // idx (NULLABLE -- required, but ancestor optional)
```

It follows from the desugaring that

- when `'address'` is present (such as `shape('city' => 'London', 'state' => 'NY', 'zip' => 'SW1')`), pattern `?'address' => shape('city' => $city, ...)` contributes to the environment the binding `$city = 'London'`.
- When `'address'` is absent, `$t2 = dict[]` and `Shapes::idx(dict[], 'city')` returns `null`, so the contribution to the environment is `$city = null`.

## Correspondence between typing and runtime

Each [typing judgment given above](#typing-judgments) has a direct counterpart in the runtime semantics, so an induction proof shows our correctness property: that the value assigned to a variable by the runtime in destructuring assignments matches the type assigned by the type checker.

**Semantic Soundness.**\=def If `G |- (p = e) -| G'` is derivable and `e` evaluates to a value `v \in [[T]]` where `G |- e : T`, then executing the desugared code of `p = e` on value `v` binds each variable `$x \in dom(G') \ dom(G)` to a value `v_x \in [[G'($x)]]`.

We have an [executable proof on github](https://github.com/mheiber/lapes). Note that the Lean development uses `$_` instead of wildcard (`_`): a minor difference that actually makes correctness harder to show.

### Worked correspondence: Example 1 (flat)

```
// T = shape('x' => int, ?'y' => int)
shape('x' => $x, ?'y' => $y) = $point;
```

| Step | Typing | Runtime |
| :---- | :---- | :---- |
| Top-level | DESTRUCTURE: exact | `desugar`: EXACT |
| Field `'x'` | FIELD-REQ \[exact\] \-\> VAR-EXACT: `$x : int` | E-Req-Var (EXACT): `$x = $t['x']` |
| Field `?'y'` | FIELD-OPT \-\> VAR-NULLABLE: `$y : ?int` | E-Opt-Var: `$y = Shapes::idx($t, 'y')` |

Typing assigns `$x : int` \-- runtime produces `$t['x']` which has type `int`. Typing assigns `$y : ?int` \-- runtime produces `Shapes::idx($t, 'y')` which has type `?int`.

### Worked correspondence: Example 2 (nested optional)

```
// let $person: shape('name' => string, ?'address' => shape('city' => string, ?'zip' => string)) = EXPR;
shape('name' => $name,
      ?'address' => shape('city' => $city, ?'zip' => $zip)) = $person;
```

| Step | Typing | Runtime |
| :---- | :---- | :---- |
| Top-level | DESTRUCTURE: exact | `desugar`: EXACT |
| `'name'` | FIELD-REQ \[exact\] \-\> VAR-EXACT | E-Req-Var (EXACT) |
|  | `$name : string` | `$name = $t1['name']` |
| `?'address'` | FIELD-OPT: mode \-\> nullable | E-Opt-Nested: mode \-\> NULLABLE |
| (access) | (inner typed at `S`, not `?S`) | `$t2 = Shapes::idx($t1, 'address', dict[])` |
| `'city'` (under nullable) | FIELD-REQ \[nullable\] \-\> VAR-NULLABLE | E-Req-Var (NULLABLE) |
|  | `$city : ?string` | `$city = Shapes::idx($t2, 'city')` |
| `?'zip'` (under nullable) | FIELD-OPT \-\> VAR-NULLABLE | E-Opt-Var |
|  | `$zip : ?string` | `$zip = Shapes::idx($t2, 'zip')` |

The mode transition happens at the same point in both systems: `?'address'`. Above it, `$name` gets exact type `string` and direct access `$t1['name']`. Below it, `$city` and `$zip` get nullable types and `Shapes::idx` access.

Typing assigns `$city : ?string` because FIELD-REQ passes `string` in nullable mode and VAR-NULLABLE wraps it to `?string`.

Runtime produces `Shapes::idx($t2, 'city')`, which returns `string` when `$t2` is the actual address shape, and `null` when `$t2` is `dict[]`.

The value is always in `[[?string]]`.
