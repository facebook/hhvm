Authors: Max Heiber, Sinan Cepel
Status: Accepted and Implemented
Internal context: [https://fburl.com/hack\_named\_parameters\_proposal](https://fburl.com/hack_named_parameters_proposal)

# Named Parameters: HIP

## Goals and Constraints {#goals-and-constraints}

### Goals

Listed in descending priority order (we plan to deliver on both):

* G1: DevX for **both humans and AI**  
  * Why:  
    * Lack of named parameters makes WWW code hard to work with: users must manually deal with large parameter lists or with the awkwardness of simulating named parameters with shapes.  
    * Our ergonomics has fallen behind that of other languages  
    * Part of a broader effort to make WWW easier for **both humans and AIs** to work with (both reading code and writing code)  
    * See also [Example of improved ergonomics for py2hack](https://fburl.com/gdoc/hsscchpg)  
* G2: increase safety  
  * Why:  
    * Prevent SEVs and related issues (including runtime errors and unexpected behavior). Specific SEVs are in https://fburl.com/hack_named_parameters_proposal
    * Support AI-generated code efforts through enabling more trust in tooling

### Hard Constraints

* C1: Easy to adopt. Implies:  
  * Ergonomics  
  * Codemoddability  
  * Converting a function and its call sites from simulating-named-params-using-shapes to real named parameters should be perf-positive  
* C2: Feasible.  
  * We need to be able to deliver value in 2025

### Defeasible Constraints:

* C3: Avoid prematurely restricting the design space  
  * We ship early but make it as straightforward as we can to add or change things later.

### Non-goals

* Improve performance significantly  
  * Why: Improving on simulating-named-params-via-shape-passing is sufficient to meet our goals and the design space is already heavily constrained.  
* Feature-completeness:  
  * Why: we can provide value and gain learnings earlier by shipping something that meets the goals but leaves open room for more features (see **Potential improvements outside initial MVP scope** below).

## Solution Sketch {#solution-sketch}

- When a parameter is named, the call site must provide the corresponding argument name.  
- Named parameters are distinct from positional parameters (parameters are not "named by default")  
- Parameter names for named parameters are part of the function type.

Examples:

```php
// definition
function foo(int $a, int $b = 0; int ...$args, named int $c, named int $d = 0): void {}

function main(): void {
    foo(1, c=2);
    foo(1, 3, d=2, c=2);
    foo(1, 3, 4, c=2, d=2);

    // hh error: too few arguments
    // HHVM RuntimeException
    foo(); 

    // hh error: too few arguments
    // HHVM RuntimeException
    foo(1);

    // hh error: unexpected named argument
    // HHVM RuntimeException
    foo(1, missing=2);
}
```

Hierarchies:

```php
class A {
    public function m(named int $a): void {}
}

class B extends A {
    // hh error: `m` must have a named parameter `$a` since it overrides `m` in class `A` which has named parameter `$a`
    // Runtime error: `m`'s override declaration is inconsistent with base.
    <<__Override>>
    public function m(named int $b): void {}

}

function main(): void {
  let $b: A = new B();

  // HHVM RuntimeException
  $b->m(a=1);}
```

Anonymous functions and types:

Note: `optional` is [an existing keyword in function types](https://fburl.com/workplace/8el6ohb9)

```php
function take((function(int, named int $b, named optional int $c)) $f): void {}


function main(): void {
    $f = (int $a, named int $b, named int $c = 0) ==> $a + b + c;
    $f(0, b=1);
    $f(0, b=1, c=2);
    take($f);
}
```

## User-facing Documentation {#user-facing-documentation}

You can declare parameters of a function as `named`, in which case callsites must specify the parameter by name instead of positionally.

```php
function add_to_table(named int $row, named int $col): void {
    // ....
}

.....

function main(): void {
    add_to_table(row=1, col=2);
}
```

In the example above, named parameters help callers avoid mixing up `$row` and `$col`:

    `add_to_table(2, 1); // hh error: missing named parameters $row and $col`

Named parameters can make your code safer and easier to read when a function has parameters that would otherwise be easily confused:  
\- multiple parameters of overlapping types (e.g. boolean flags, ?int and null)  
\- especially when such parameters have default arguments

Here’s an example with easily-confused parameters:

```php
 protected async function genMessageDelta(
    Context $context,
    ID $id,
    Map<string, string> $data,
    ?arraykey $offline_id = null,
    ?vec<string> $sender_tags = null,
    ?vec<string> $meta_tags = null
)
.......

await genMessageDelta($ctx, $id, null, null, vec['tag1']);
```

By making the easily-confused parameters named, calls to the function get both safer and easier to read:

```php
 protected async function genMessageDelta(
    Context $context,
    ID $id,
    Map<string, string> $data,
    named ?arraykey $offline_id = null,
    named ?vec<string> $sender_tags = null,
    named ?vec<string> $meta_tags = null
)
.......

await genMessageDelta($ctx, $id, meta_tags=vec['tag1']);
```

Named parameters behave just like positional ones except for how you refer to them at callsites: named parameters can have default arguments, and are matched according to their names at call sites. 

Note that args for named parameters can be interspersed with positional args :

```php
function send(
  string $message, 
  named string $host = 'www.thefacebook.com',
  int $retries = 3 // default argument
) { 
...
}


async function foo() {
  send('poke', host='www.facebook.com');
  // can permute order
  send(host='www.facebook.com', 'poke');
}
```

#### Historical remark

You may see older code that simulates named parameters using shapes:

```php
 protected async function genMessageDeltaShapeVersion(
    Context $context,
    ID $id,
    Map<string, string> $data,
    shape(
     ?'offline_id' => arraykey,
     ?'sender_tags' => vec<string>,
     ?'meta_tags' => vec<string>,
    ) $opts
) {
  $offline_id = $opts['offline_id'];
  $sender_tags = $opts['sender_tags'];
  $meta_tags = $opts['meta_tags'];
  // ...
}
.......

await genMessageDeltaShapeVersion($ctx, $id, shape('meta_tags' => vec['tag1']));
```

Prefer using named parameters over simulating-named-parameters-via-shapes: Names parameters are:  
\- more ergonomic  
\- more performant  
\- safer: HHVM checks types for named parameters but not for individual keys in a shape in parameter position

## Solution in Detail {#solution-in-detail}

### Syntax

\> Syntactic restrictions are enforced by both HHVM and Hack, which share a parser  
\> For syntax at a glance, see [examples](?tab=t.0#bookmark=id.hxe2kyrt1ja5) above

In declarations:

- Named parameters are indicated with the `named`  keyword.  
- Excerpt from how we will update the parser, the new part is indicated. (note that our current BNF-style comments in the parser are out of sync with the impl, so impl is more informative here):  
  - [parameter\_declaration before]($REPO//hphp/hack/src/parser/core/declaration_parser.rs): `attrs_opt  visibility_opt  inout_opt  readonly_opt ellipsis_opt <ellided>`  
  - parameter\_declaration after   : `attrs_opt  visibility_opt  inout_opt named_modifier_opt  readonly_opt ellipsis_opt <ellided, no change>`  
  - `named` can be used in constructors and is allowed with promoted properties: `class A { function __constructor(public named int $x)`  
- Named parameters can have default arguments: function foo(named int $x \= 1): void {}  
  - Same restrictions (and freedoms) on default arguments for named parameters as for positional parameters  
- Named parameters will initially not support inout [for pragmatic reasons to do with the runtime](https://fburl.com/gdoc/lki55ag2). Such support may be added in future.

In call sites:

- Basic syntax: `foo(x=3)` supplies argument `3` for named param `$x`  
  - Why: [Named Parameters HIP supplement: why = for separating param names from arg values in function calls](https://fburl.com/gdoc/jmme24ua)  
- Positional arguments can be interspersed with named arguments: `bar(y=5, 1, 2, 3, x=4)`

Named parameters in type syntax:

* Named parameters are indicated with the `named` keyword, which comes before `optional` and before the type of a parameter. `function take((function(int, named int $b, named optional int $c)) $f): void {}`  
* Named parameters must have names.

### Runtime Semantics and Checks

A complete description of the runtime behavior is in the Supplement: Runtime Design section below.

#### Basic representation

- The runtime will extend Func objects to store the names of named parameters, and store them internally in an implementation-defined order, distinguished from positional parameters.  
- The calling convention will be extended such that:  
  - Named parameters are passed from the caller to the callee respecting the implementation-defined order of names, including an additional token containing the names of the parameters to deal with the case of dynamic and unknown calls.  
  - The callee will perform the various parameter name and type hint checks, detailed below, and populate default values as appropriate. A runtime exception will be thrown if a required name is missing or an argument name not matching a parameter is provided.  
- Type hints and behavior named parameters will be consistent with positional parameters except for the additional name checks.

#### Order of argument evaluation

Arguments will be evaluated left to right (based on call site). Specifically, for a callsite `foo(b=expr_b(), a=expr_a())` , `expr_b()` would be evaluated first even if the implementation-defined ordering would pass `a` before `b` to the callee.

Why:

- Likely best DevX (G1), users have an intuition of evaluation being left-to-right. Additionally, every other language we’ve checked evaluates args-for-named-params in left-to-right order (except OCaml, where order is not specified). See column D13 in the Supplement: Named Parameters Prior Art and Design Space section below.  
- HackC doesn’t have access to decls, so ordering based on the parameter order in the declaration is not really on the table by [C2](?tab=t.0#bookmark=id.cifcypqlf7c7) (feasibility constraint)

### Callee-side runtime checks

#### Type checks

On the caller side, type checks for named parameters will be performed analogously to positional params. Type hints will be checked in the order of (1) positional parameters, (2) named parameters and (3) all reified types.

Type hint checks for positional and named parameters will be consistent in the checks performed, but for named parameters, the checks will *not* necessarily be in the user-declared order. Type hint error messages will be issued in an order consistent regardless of which order they were declared in.

#### Callsite arity & name checks

HHVM behavior for positional parameters is unchanged:

- HHVM will produce a `RuntimeException` when a required positional parameter is not provided.  
- HHVM will not produce an exception for extra parameters passed positionally. This is arguably a mistake, but we're not changing it here.

HHVM behavior for named parameters:

- `RuntimeException` for not providing a named argument for a non-optional named parameter  
  - Why:  
    - by G2 (safety goal). If we don’t do this check, it’ll be behavior changing for developers to add new named parameters.  
    - by [C3](?tab=t.0#bookmark=id.fcysferp7pxg) (avoid prematurely limiting the design space). If we don’t do this check, it will be harder for us to add shape-splatting for named parameters in future.  
- `RuntimeException` for providing a named argument that doesn't correspond to any named parameter.  
  - Why:  
    - by G2 (safety goal) we want it to be safe for API designers to add new named parameters in the future without breaking existing code  
    - by [C3](?tab=t.0#bookmark=id.fcysferp7pxg) (avoid prematurely limiting design space) we want to allow for potentially adding named variadics in future (on analogy with positional variadics, may resemble Python \*\*kwargs).

#### Hierarchy checks

We propose introducing override compatibility checks in the runtime. If a class attempts to override a method with a new method that doesn't provide all named parameters or overrides a named parameter with-default into one without, HHVM will throw a RuntimeError.

### Named Parameters in Type Structures

Reified types for functions containing named parameters will include the name of the parameters in the type structure. Named parameters for function types in type structures will be laid out after all positional parameters in an implementation-defined order, regardless of the declaration order, and the name will be retained in the type structure.

### \_\_Memoize, \_\_MemoizeLSB

Functions with named parameters will be memoizable. Memoization keys will depend on the set of parameter names, but will not be affected by the declaration ordering. Changing the order of named parameters in the declaration won’t change the memoization key.

### hh

* Named parameters are included in types.

See also [example above](?tab=t.0#bookmark=id.n58g1kbhzq4l).

*Note for Hack team: named parameters should be explainable in terms of a [purely-theoretical-but-not-actual rewrite to code that passes shapes](https://fburl.com/gdoc/69roa5fa), for migratability and language feature composability.* See also https://github.com/mheiber/lapes for a formalization.

Function subtyping rules follow from the rewriting semantics, an example is:

```java
   (function(arraykey, named arraykey $b, named optional arraykey $c))
<: (function(int    ,  named int      $b, named optional int      $c))
```

In addition to checking types:

- hh errors when a named argument is missing (when there is no named argument at the call site corresponding to a named parameter in the callee)  
- hh errors on extra named arguments (named arguments that don’t have a corresponding named parameter)  
- hh errors on incorrect overrides  
  - Note: names of named parameters are part of function types

In addition, `hh` will reflect the [order of evaluation of arguments](?tab=t.0#bookmark=id.bqam4o1hu2hi) for:

- type refinement (such as refining from ?int to int)  
- type invalidation (due to side effects)

## Prior Art and More Design Justification {#prior-art-and-more-design-justification}

See the Supplement: Named Parameters Prior Art and Design Space section below.

## Potential future work {#potential-future-work}

See the Supplement: Potential Future Work section below.

## Supplement: Runtime Design

Main HIP: See the Named Parameters: HIP section above.

#### Runtime constraints

We plan on named parameter semantics to be explainable via elaboration-into-shapes. i.e. 

```
// Declaration
foo(named int $x, named int $y = 2) { ... }

// Can be thought of as
foo(shape(‘x’ => int, ‘y’ => int) $s) { $x = $s[‘x’]; $y = $s[‘y’]; … }

// Use
foo(x:1, y:2);
// Elaborated to
foo(shape('x' => 1, 'y' => 2));
// Reordering is OK
foo(y:2, x:1);
```

This elaboration into shape has some relevant consequences for us:

* In general, we won’t know the position of a named parameter but we need to be able to match it to the right param  
* Positional parameters are treated distinctly from named parameters, i.e. named params are defined by their name rather than their position, and should not interact with positional parameters. As an example, it should be possible to interleave named parameters anywhere in the declaration without affecting how positional parameters or variadic parameters are laid out.  
* We can’t rely on a consistent order for things like defaults in the general calling convention.

We have some additional checks we want to have for future-proofing:

* HHVM should throw a RuntimeException if an unexpected parameter is passed in.  
* HHVM should throw a RuntimeException for a missing named parameter without a default.  
* HHVM should throw a RuntimeException when overriding a base method with incompatible named parameters (missing a named parameter that exists in the base method, or lacking a default value for a named parameter that has a default in the base).

#### Representation changes

Functions have an existing shared ParamInfo object that’s already 88 bytes in non-lowptr: [$REPO//hphp/runtime/vm/func.h]($REPO//hphp/runtime/vm/func.h)

I propose adding a new “Named” flag to parameters (still fits in the uint8\_t) and adding an extra `LowStringPtrOrId` field for the parameter name.

#### Calling convention with named parameters

See [https://fburl.com/excalidraw/f2l4khnn](https://fburl.com/excalidraw/f2l4khnn) for details.

#### Inout Named Parameters

We propose banning inout named parameters for the MVP. The problem they pose from a runtime perspective becomes clear when considering how regular inout parameters are handled:

For existing inout params, the caller specifies a count of inout parameters when calling, and creates extra cells for the right number of inout values being returned. The callee will `RetM` with the appropriate number of extra return values.

For named inout params, we have two choices: 

1. We could ignore the names, and rely on the consistent runtime ordering observed elsewhere (i.e. if you have `inout $a` and `inout $b`, have a convention of always ordering the cell for `$a`’s value before `$b`’s cell), and throw a RuntimeException if the number of cells returned don’t match the cells pushed by the caller. This would be more performant, but have a safety downside: If a dynamic or unknown call has a separate set of names for the inout named parameters (with matching count), we’d be silently propagating incorrect values.  
2. We could validate the returned names matching the inout names on the callee side. One potential design here would involve the caller placing a C-style array with the inout names before the inout cells (in addition to the set of all parameters), and the callee validating that structure in the prologue, and use the design of (a) for the rest of the checks. This would be a significant tax on calls with inout params, but that may be acceptable depending on the frequency of calls.

#### Memoize

Memoization will be handled transparently.

#### Initializers

We will initially not support initializers. There’s a couple options on how to support it moving forward:

1. We can try directly inlining the bytecode for initializers/call the 86cinit functions in prologues.  
2. We can have a special bytecode section that keeps the bytecode needed to initialize each constant, have the prologue jump to those.

#### Splatting

We will have a separate unique stub for splatting.

## Supplement: Potential Future Work

The following could be added after we release the features described in the Named Parameters: HIP section above.

These are a combination of:

* We have some nice-to-haves that can be added after we’ve addressed our main goals. These include likely-desirable features, but we’d like to focus on getting an extensible basic design first and relying on user feedback to prioritize features.  
* Things in the design space we don’t want to prematurely close off but are not considering any time soon

This work includes:

* Tooling:  
  * lint+codemod requiring parameters to be named in cases that would otherwise be footgunny. We can prevent future SEVs (like those listed in Goals above) using a lint that nudges toward “use a named parameter here” at otherwise-footgunny function definitions..  
  * quickfix for making a parameter named  
* [Punning](https://dev.realworldocaml.org/variables-and-functions.html#labeled-arguments) may make the DevX (goal G1) *even better* , but isn’t strictly necessary for better devX and safety, so we defer it.  
* Enable [shape splatting](https://fburl.com/gdoc/tfn2wohw), similar to how we now allow splatting tuples into positional arguments.   
* Named generic parameters: dict\<Key:string,Value:int\>  
  * Not much interaction with the current proposal, best considered in separate proposals imo, if it turns out there are use cases.  
* See also the Supplement: Named Parameters Prior Art and Design Space section below: we indicate more future extensions and their advantages and disadvantages.

## Supplement: Named Parameters Prior Art and Design Space

### Column Key

- **D1**: When a parameter is named, giving the name at call sites is required or optional?
- **D2**: A child class can override a method and provide different names for its arguments
- **D2.1**: Type-checker checks by (note: subtle due to overridden methods)
- **D2.2**: Runtime checks (note: subtle due to overridden methods)
- **D3**: Can reorder named arguments at call site
- **D4**: Can define internal parameter name distinct from external parameter name
- **D5**: All parameters are "named" by default
- **D6**: Method ptr type includes param names for named params AND named params allowed in lambdas
- **D7**: Punning
- **D8**: Splatting
- **D9**: Parameter name available via facts API
- **D10**: Syntax
- **D11**: Restrictions on order of params
- **D12**: Named parameter version of variadics (python kwargs)
- **D12b**: Inout parameters
- **D13**: Evaluation order for arguments corresponding to named parameters

| Language | D1 | D2 | D2.1 | D2.2 | D3 | D4 | D5 | D6 | D7 | D8 | D9 | D10 | D11 | D12 | D12b | D13 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| disadvantages |  | - ergonomics (complicated, potentially unintuitive)<br>- may lead to unsafety (unless we manage with restrictions) |  |  | (1) leads to inconsistency between call sites. (2) May make it less obvious to AIs and people how to use the function (3) reduces correspondence between the call site and the definition (which, fwiw, I find convenient and intuitive) | - noisy, verbose, and (imo) hard to read and remember<br>- LLMs seem to have trouble figuring out how to correctly call functions that have separate internal and external parameter names | - requires codemodding the world, especially for overrides that change parameter names vs the parent<br>- moves the language away from one way to do things |  |  | Multiple overlapping splats and potential splat/explicit naming overlaps are a footgun in other languages that contradicts G1 | n/a |  |  |  |  |  |
| advantages |  | - adoptability for existing code<br>- refactorability<br>- more specialized names may make more sense in subclasses |  | By C3 (avoid prematurely restricting design space): adding checks before a feature is adopted is way easier than adding after. We can always loosen restrictions later if we need to.<br><br>By C1 (DevX): HHVM failing fast rather than doing something weird imo helps users pinpoint problems faster. | - Andrew Kennedy has pointed out that it would be nice to explain the semantics in terms of shapes (even though we won't implement that way). Advantages I see is that such a "desugaring semantics" pushes toward consistency, make it more likely that we are set up well for splatting to work sensibly, and will make it more likely that we can accomodate existing use cases for shape-passing (providing a better alternative).<br>- Given that we include names for named parameters in function types, allowing reordering makes it more things compatible. It could be annoying and wasteful to have to write adaptor functions to convert from a function `(named int x, named int y)` to (named int y, named int x)` or to duplicate interfaces. | Note: It's always possible to simulate this capability using local variables.<br>- can help devs avoid lints for unused parameters in function definitions. The workaround on the current proposal is `function foo(named int $x): void { $_ = $x; }`. <br>- can make it easier to support method overriding s.t. names have to match: we could make it so only the external name needs to match.<br>- Gives a way to access things in an outer scope without changing the outer scope. Example with internal-vs-external-names P1881417913 and a workaround: P1881416688 | Enables callers to opt in to more safety for parameters not explicitly marked as named |  | - reduces boilerplate, sometimes making Hack code easier to write and read | Makes taming larger signatures easier, helps G2 | Makes it easier (possible?) for codemods to generate correct code |  |  |  | byG1 (DevX for AI and for humans):<br>in particular it seems to be easier for AI to generate code when features compose and stuff just works |  |
| reversible/relaxable decision | Name is required | No | No (would be a big change) | No (would be a big change) | Yes | No | No | Yes | No | No | Yes, helps with G2 |  | Yes | Yes | ? |  |
| our recommendation. | Yes, by G2. | No for MVP, by G1, C2, maybe G2 | static type. Which will match the runtime type, due to the decision in previous column (no overriding and changing name).<br><br>hh will reject:<br>- not providing a named argument for a non-optional named parameter<br>- providing a named argument that doesn't correspond to any named parameter<br>- overriding a method incorrectly (remember that the names of named parameters are included in function types) | by method declared in runtime class (following the same rules of dynamic dispatch in Hack. Note that this is distinct from languages like Scala where *static class* determines which names are used for named parameters).<br><br>HHVM will reject:<br>- not providing a named argument for a non-optional named parameter<br>- providing a named argument that doesn't correspond to any named parameter<br>- overriding a method incorrectly (remember that the names of named parameters are included in function types) | Yes | No for MVP, by G1. In experiments with LLMs, they were unable to infer how to call a function correctly when it had different internal-vs-external params (and, fwiw, I often forgot the syntax for separating internal-vs-external names in OCaml and Objective-C) | No for MVP by C1 and C2.<br>Unlikely for follow-on work, since it would be confusing to have both named-or-positional args and named args. Python has both kinds (actually, at least three kinds) but in a way that is unsafe | Yes, needed for typed mocking. We want to support typed mocking by G2 (safety) even though this makes implementation harder (tension with C1).<br>See Named Parameters: Goals and Constraints | No for MVP, not needed for G1 or G2 and in slight tension with C1 (C1 implies codemoddability, but punning introduces a minor+mitigatable refactoring hazard) | No for MVP, by feasibility constraint C2 | No for MVP, by feasibility constraint C2 saving for follow-on | provisional syntax:<br><br>```<br>function foo(named int $x): void {}  <br>...<br>foo(x:$x) // PHP syntax<br>```<br><br>Syntax chosen because LLMs predicted the call syntax correctly given the decl syntax and an LLM recommended this `named` keyword solution. | intuition: positional before named<br><br>restriction:<br>positional+required, followed by positional+optional, positional+variadic, followed by named<br><br>Note: we need to relax the current rule that variadic has to come last<br><br>Allowed:<br><br>function foo(int $a, int $b = 1, named int $c, named int $d = 1)<br><br>Disallowed<br>function foo(int $a, int $b = 1, named int $c = 1, named int $d)<br>function foo(int $b = 1, named int $c = 1, named int $d, int $a)<br>.....<br><br>Allowed but counterintuitive:<br><br>function foo(named int $x = 0, named int $y = 1) {}<br>function bar() { foo(y:2); }<br><br>Why:<br><br>By C3 we do not want to block future features. This seems to me to leave things maximally open, including enabling reordering arguments (D3), maybe allowing named-args-everywhere, fast-calling-convention under the hood, etc. | No for MVP by feasibility constraint C2.<br><br>Two potential follow-ons:<br>- enable splatting shapes<br>   - Note: for consistency and to avoid bad incentives, we would probably then also allow reordering of named arguments, since shape types do not specify order<br>   - Note: unsure of performance implications here<br>- enable splatting named tuples<br>   - Note: would require named tuples to exist in Hack<br><br>Use cases may include Ents needs. Currently they use splatting along with enum classes and variadics to avoid code bloat. | Will support if there's an efficient way to do so in the runtime |  |
| Scala <br><br>Scala spec<br>Scala example 1 | Optional | Yes | Parameter name in the class given by the static type example.<br><br>In Scala, since overriding methods can have default values, arguments at the same position can match with distinct parameters (by position) when overriding (example). Seems like a safety hazard. We could address the hazard by not allowing such ambiguous-looking cases, but this does require propagating a lot of information. A way of partially addressing is to disallow permutation. | None (90% sure) | Yes, with restrictions (D11) | No | Yes | ? | ? |  |  |  |  |  |  | left-to-right by call site: https://scastie.scala-lang.org/sM75rKoBQ3eEno3wLQY32Q |
| Kotlin <br><br>Kotlin Spec<br>Kotlin example 1 | Optional | Yes | Parameter name in the class given by the static type: example<br><br>In Kotlin, an overriding method cannot have default arguments (example). This avoids ambiguities but is counter to how Hack works and could hinder adoptability. | None (90% sure) | Yes, with restrictions (D11) | No | Yes | ? | ? |  |  |  |  |  |  | left-to-right by call site: https://pl.kotl.in/4bCPiojcz |
| Swift<br><br>Swift reference<br>Swift example | Required | n/a, see next column | n/a, since parameter name is part of the method name. Also: Can get a compiler error by using ‘override’ keyword in a function definition with the same signature except for parameter name. Also Swift is Yes for D4 (internal-vs-external names) | n/a, since parameter name is part of the method name | No<br><br>example | Yes | Yes | ? | ? |  | n/a |  |  |  |  | left-to-right by call site |
| C#<br>C# spec<br>C# example | Optional | Yes | Name in the overridden method (even if the static type is from the parent class) | ? | Yes, with restrictions (D11) | No | Yes? | ? | ? |  | n/a |  | TODO |  |  | left-to-right by call site |
| Python (mypy), but note that mypy probably violates the spec<br><br>Canonical references for typing:<br>- Typing spec<br>- Typing conformance suite | Optional unless defined as “keyword-only” | Yes | By static type (unsound!) | By runtime type | Yes, with restrictions (D11) | No | Yes, unless defined as positional-only | Yes, optionally (80% sure) | No |  | n/a |  | TODO | Yes<br><br>def foo(a: int, b: int): ...<br><br>kwargs = {'a': 1, 'b': 2}<br>foo(**kwargs) |  | left-to-right by call site |
| Python (pyre) |  |  |  |  |  |  |  |  |  |  | n/a |  |  | same as cell above |  | same as cell above |
| Python (pyright) |  |  |  |  |  |  |  |  |  |  | n/a |  |  | same as cell above |  | same as cell above |
| Ruby<br><br>Ruby docs<br>Ruby example | Required | Yes | n/a | By runtime type | Yes, with restrictions (D11) | No | Yes | n/a | No |  | n/a |  | TODO |  |  |  |
| Ocaml<br><br>OCaml manual<br>OCaml example<br>OCaml syntax cheat sheet | Required (when setting the warning to be an error, which in practice I think most professional codebases do) | No | n/a: overridden method must have same names for named parameters | n/a, not checked at runtime (95% sure) | Yes, with restrictions (D11) | Yes<br><br>Example:<br><br>let add_1 ~x:y = y + 100<br><br> - external name is `x`<br> - internal name is `y` | No | Yes | Yes |  | n/a |  | TODO |  |  | unspecified |
