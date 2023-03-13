# Memoization options

This document describes proposed changes to the already defined Implicit Context feature in pursuit of the following goals:

* Expedite wide-scale usage of Implicit Context (IC)
* Ensure IC is always properly accounted for in memoization
* Reduce risk of breaking runtime assertions as part of initial migration or normal development
* Reduce risk of breaking existing logic depending on behavior of `<<__Memoize>>` today

> This feature is also referred to as the "Dynamically Enforced Implicit Context" feature.

## Today: IC requires Contexts and Capabilities

The Implicit Context feature provides a value that is implicitly propagated to callees recursively. For example:

```
// Native, in HH namespace
abstract class ImplicitContext {
  abstract const type T as nonnull;
  public static function set<Tout>(
    this::T $context,
    (function ()[zoned]: Tout) $f
  )[zoned]: Tout { ... }
  public static function get()[zoned]: ?this::T;
}

// Userland
final class MyPolicyImplicitContext extends HH\ImplicitContext {
 ...
}
```

```
MyPolicyImplicitContext::set(
  $my_policy,
  ()[zoned] ==> do_stuff(),
);

...

function some_recursive_callee_of_do_stuff()[zoned]: void {
  $_ = MyPolicyImplicitContext::get(); // returns $my_policy
}
```

In order to prevent poisoning memoization caches (fetching the cached results computed under one IC value when executing under a different IC value), we designed the IC feature to be coupled with the coeffects feature which applies static recursive restrictions. We required that:

* You can only set an IC for the execution of functions requiring at most `[zoned]` (or `[zoned_with<T>]` but we ignore this context in this document)
* Memoized functions that are callable from a `[zoned]` function must either:
    * shard the memoization cache based on the IC value (iff the context of the function is `[zoned]`)
    * guarantee that the IC is not used by having a context without the `ImplicitPolicy` capability which is required to access the IC (e.g. `[leak_safe]`)

While tying the IC to contexts and capabilities gives us static guarantees about code, adding these more restrictive contexts to code requires a lot of effort, and some developers want the benefits of the IC without the requirement of using contexts.

## Proposal

We propose allowing setting an IC when executing functions requiring `[defaults]`. The new signature for this method on the `ImplicitContext::` class will be:

```
  public static function runWith<Tout>(
    this::T $context,
    (function ()[_]: Tout) $f
  )[ctx $f, zoned]: Tout { ... }
```

>In the above, we have also renamed `ImpicitContext::set` to `ImplicitContet::runWith` in order to avoid an incorrect assumption that the IC is set to the given value *after* the completion of this method call. We use the new “runWith” name for the remainder of this document.


We will avoid poisoning memoization caches by dynamically requiring that memoized functions executing under an IC fall into one of two safe categories:

* The function is marked as having its memoization cache key incorporate the IC. This is achieved by using the `__Memoize(#KeyedByIC)` attribute (formerly known as `__PolicyShardedMemoize`) instead of regular `__Memoize`. We will allow this attribute to be used with any function with the `ImplicitPolicy` capability including `[defaults]` functions.
* The functions is marked as never using the IC. This can be achieved either by:
    * Using the new attribute `__Memoize(#MakeICInaccessible)` instead of regular `__Memoize`. This attribute will act like `__Memoize` except: attempting to fetch the IC or calling an uncategorized memoized function from a function with this attribute will throw. This behavior applies to immediate and recursive callees until an IC value is set again (if ever).
    * Using regular `__Memoize` and requiring a context that does not have the `ImplicitPolicy` capability. Because fetching the IC requires `[zoned]`, we know that recursive callees cannot access the IC. (Given the current set of contexts, this is the set of contexts as capable as or less capable than `[leak_safe, globals]`.)

>In the above, we are adding an optional enum class label argument to the `__Memoize` attribute. See the “Syntax” section.


If a memoized function is executed under a non-null IC and does not fall into one of the above two categories, the runtime will throw an exception. The primary example of such a function would be a function that requires `[defaults]` and has the regular `__Memoize` attribute.

We also propose adding the following features to allow for migration to the above while minimizing the risk of surprise exceptions:

* `soft_run_with($callable, $blame_string)` - This function will execute the callable and log in the same circumstances when calling `runWith` instead would throw: namely when calling a function that is `__Memoize` + `[defaults]`. The blame string distinguishes distinct uses of this API. Developers can use this function before using `runWith` in order to identify functions to fix.
    * Note that this function does not belong to an `ImplicitContext` class because it does not effect propagation of a user-provided value.
* `__Memoize(#SoftMakeICInaccessible)` - This attribute is analogous to `__Memoize(#MakeICInaccessible)` and will cause logs in cases when using `__Memoize(#MakeICInaccessible)` instead would throw: namely when calling a function that is `__Memoize` + `[defaults]` or when attempting to fetch the IC. Developers can use this before using `__Memoize(#MakeICInaccessible)` in order to identify functions to fix.
    * Note that since a function with `[defaults]` and this attribute behaves exactly like a function with `[defaults]` and `__Memoize` except for the logging, a function with this attribute will also be considered "uncategorized."
    * When using `#SoftMakeICInaccessible`, the attribute will also take an integer as an optional second argument. This integer will be used as a sample rate. That is, if you pass N, then when this function is called there is a 1/N chance that we will enter the “soft inaccessible” state and produce logs; otherwise, this function will act as if no arguments we passed.
* `run_with_soft_inaccessible_state($callable, $blame_string)` - This function will execute the callable and log in the same circumstances when calling this from a `#MakeICInaccessible` function would throw. The purpose of this function is to allow removing calls to backdoors in code that is already running under an “inaccessible” state.
    * This currently will only be allowed from the “null” state which is the state you are in in a backdoor.

The goal of logging is to allow developers to get signal if making changes would result in exceptions due to disallowed access to the IC or calls to uncategorized memoized functions. Developers who use these "soft" variations should get at least one log for problematic dependencies if any exist, even if other callers of the same dependencies exist (assuming 1:1 sampling). Note that while getting logs for every problematic dependency that could be detected at runtime is ideal, logs for a single dependency should be sufficient to block the developer from upgrading to throwing variants.

These various memoization options are mutually exclusive and cannot be used together on the same function.

Note: In this document, we describe semantics in terms of `__Memoize` and `__Memoize()`. The same statements apply to `__MemoizeLSB` and new `__MemoizeLSB()`.

## Description of states

Under this proposal, the IC can be in one of five states:

```
null : The IC has not been set
value(T) : The IC has been set to some value
inaccessible : Fetching the IC or calling uncategorized memoized functions will result in an exception
soft_set(vector<soft inaccessible blame>, vector<soft run blame>) : The IC has not been set, but you will get logs for uncategorized memoization
soft_inaccessibl(vector<soft inaccessible blame>, vector<soft run blame>) : The IC has not been set, but you will get logs for uncategorized memoization and for fetching the IC
```

Both soft states maintain two vectors of information on caller frames: one for strings associated with the reason for entering a “soft inaccessible” state (usually the names of functions with `__Memoize(#SoftMakeICInaccessible)`), the other for the list of user-provided strings passed to calls to `soft_run_with`. They are used as in two places:

* Functions with `__Memoize(#KeyedByIC)` will account for both vectors in the memoization cache key.
* Warnings that are raised for calling uncategorized memoized functions (from either soft state) or fetching the Implicit Context (from `soft_inaccessible`) will not include the blame information in the error string. However, this blame information will be passed as an additional argument to the error handler — a 2-tuple of the two vectors of strings.

The "blame" values help indicate what function calls or functions' attributes are responsible for the logs. This is necessary in order to address logs during migrations, especially when migrating multiple code-sites at once.

## Description of state transitions

The following actions will have the following behavior:

### Calling a function with regular `__Memoize` that has the `ImplicitPolicy` capability (e.g. `[defaults]`)

This does not affect the state of the IC.

```
null: Run, do not use IC for cache key
value: throw
inaccessible: throw
soft_set: log, run, do not use IC for cache key
soft_inaccessible: log, run, do not use IC for cache key
```

Note that although `[zoned]` has the `ImplicitPolicy` capability, it is an error for a function with the `[zoned]` context to use regular `__Memoize` and so is not subject to this

### Calling a function with `__Memoize(#KeyedByIC)`

This does not affect the state of the IC.

```
null: Run and use null as the IC key
value: Run and use the IC value as the IC key
inaccessible: Run and use the inaccessible state (singleton value) as the IC key
soft_set: Run and use the the state plus blame vectors as the IC key
soft_inaccessible: Run and use the the state plus blame vectors as the IC key
```

For the soft states, it is necessary to key by blame information. Otherwise, if there are multiple callers that enable logging and a callee that would produce a log, if all the callers share the same cache key, that logging callee will not produce logs for every caller and non-first callers will lose signal that the callee may eventually throw.

### Calling a function with `__Memoize(#MakeICInaccessible)`

```
* : Run, do not key cache with IC, transition to inaccessible state with this function as blame
```

### Calling a function with `__Memoize(#SoftMakeICInaccessible) `

Recall that functions with this attribute are still considered uncategorized and so have similar behavior to a `__Memoize` + `[defaults]` function.

```
null: Run, do not key with IC, transition to soft_inaccessible state with this function as blame
value: throw
inaccessible: throw
soft_set: log, run, do not key with IC, transition to soft_inaccessible state, inherit both blame vectors and append this function name to the inaccessible blame vector
soft_inaccessible: log, run, do not key with IC, stay in soft_inaccessible state and add this function to existing inaccesible blame vector
```

### Calling a function with `__Memoize` that does not have the `ImplicitPolicy` capability

This does not affect the state of the IC.

```
* : Run, do not key with IC
```

### Calling `ImplicitContext::get`

This does not affect the state of the IC.

```
null: return null
value: return value
inaccessible: throw
soft_set: return null
soft_inaccessible: log, return null
```

### Calling `ImplicitContext::runWith`

```
* : set the IC to a value, transition to the value state with this function call as blame
```

### Calling `soft_run_with`

```
null: transition to soft_set state with the given string as blame
value: no op (relevant callees will already throw)
inaccessible: no op (relevant callees will already throw)
soft_set: stay in soft_set state and add the given blame string to the existing soft run blame vector
soft_inaccessible: stay in soft_inaccessible state and add the given blame string to the existing soft run blame vector
```

Note: From the `soft_inaccessible` state, we stay in `soft_inaccessible` instead of transitioning to `soft_set` because we still want to produce warnings for downstream calls to `ImplicitContext::get` in order to associate them with the previous `__Memoize(#SoftMakeICInaccessible)` and `soft_inaccessible` has this behavior in addition to the warning behaviors of `soft_set`.

### Calling `run_with_soft_inaccessible_state`

```
null: transition to soft_inaccessible state with the given string in the soft inaccessible function blame vector
value: throw
inaccessible: throw
soft_set: throw
soft_inaccessible: throw
```

We may consider supporting this call from other states if we determine there are other uses for it.

### Calling `HH\Coeffects\backdoor` or aliases

These backdoors allow calling code requiring `[defaults]` from various, less-capable contexts.

```
* : move to null state (clears any IC value)
```

### Calling `HH\Coeffects\fb\backdoor_to_globals_leak_safe__DO_NOT_USE`

This backdoor allows calling code requiring at most `[leak_safe, globals]` from any context.

```
* : no op, no state change
```

Note that by enforcing that the code executed via this backdoor can only require at most `[globals, leak_safe]`, we prevent problematic calls to unsafe memoized functions and calls to fetch the IC.

## Attributes’ interactions with coeffects

This section describes rules for what functions with what coeffects can use these new attributes.


* `__Memoize(#KeyedByIC)` will only be permitted on functions that are known to have the `ImplicitPolicy` capability at compile time.
    * This means that the functions’ context list must be implicitly `[defaults]` or contain one of the following contexts explicitly: `defaults`, `zoned`, `zoned_shallow`, `zoned_local`.
    * Reason: A developer should be able to expect that a function without the `ImplicitPolicy` capability is not affected by the Implicit Context. However, this would not be the case if we allowed `#KeyedByIC` — e.g. imagine a memoized, leak-safe function that returns a random number.
* `__Memoize(#MakeICInaccessible)` and `__Memoize(#SoftMakeICInaccessible)` will only be allowed on functions with any of the following contexts: `defaults`, `leak_safe_shallow`, `leak_safe_local`
    * Reason: For functions without the `ImplicitPolicy` capability, the IC is already inaccessible and the function can use regular `__Memoize`. For functions with `zoned`, using this attribute seems contradictory.
    * This will be similarly enforced at compile time.
* (As before) A memoized function with the `zoned` context must use `#KeyedByIC`.
* (As before) A function with dependent contexts may not be memoized.

## Expected use

A developer that wants to execute code under an IC would:

1. Start by executing the code using `soft_run_with`
2. Address logs for uncategorized memoized functions by either (a) switch to `__Memoize(#KeyedByIC)` or (b) use `__Memoize(#SoftMakeICInaccessible)`.
    1. The latter may produce additional logs for uncategorized memoized functions and additional logs for `ImplicitContext::get`.
3. Continue to categorize detected uncategorized memoized functions AND address logs for `ImplicitContext::get`. Options for addressing `ImplicitContext::get`:
    1.  Use `__Memoize(#KeyedByIC)` instead of `__Memoize(#SoftMakeICInaccessible)`
    2.  Introduce a new IC between the `__Memoize(#SoftMakeICInaccessible)` and the `ImplicitContext::get` using another `soft_run_with` and eventually `runWith` (or by removing the IC via a backdoor function)
4. New uses of `__Memoize(#SoftMakeICInaccessible)` may produce additional logs as in step (2) and/or additional logs for `ImplicitContext::get`; Continue to categorize uncategorized memoized functions detected by logging and address
5. Harden `__Memoize(#SoftMakeICInaccessible)` to `__Memoize(#MakeICInaccessible)` when logging indicates it is safe.
6. Harden calls to `soft_run_with` to `runWith` when logging indicates it is safe.

Note that developers may also choose to use static analysis or existing runtime profiling to identify uncategorized, memoized dependencies ahead of using `soft_run_with` or `__Memoize(#SoftMakeICInaccessible)`.

Framework developers that want to make their frameworks compatible with being executed under an IC would use similar steps except likely by starting with making their regular `__Memoize` calls either `__Memoize(#KeyedByIC)` or `__Memoize(#SoftMakeICInaccessible)` depending on intended semantics.

## Path to contexts and capabilities

Allowing use of a dynamically-enforced Implicit Context is compatible with a statically-enforced Implicit Context using Contexts and Capabilities. In fact, the work to specify IC-handling for memoized functions is a subset of the work required to make functions callable from `[zoned]` contexts.

In abstract, you can think of functions using `__Memoize(#KeyedByIC)` as being functions where the intention is to eventually require `[zoned]` (if it does not already) and think of functions using `__Memoize(#MakeICInaccessible)` as being functions where the intention is to eventually require `[leak_safe]` or some more restrictive context.

## Explicit Uncategorization

We also propose another memoization option `#Uncategorized`. Passing this argument will have the same behavior described above for not passing any arguments (with the exception of functions without the `ImplicitPolicy` capability).

The purpose of this is that you can then require that the memoize attribute takes an argument. This encourages developers to consider how their function should behave w.r.t. the IC when writing new functions.

Even under such a requirement though, memoized functions without the `ImplicitPolicy` capability will continue to require that no arguments are passed to the memoize attribute as the context list already explicitly designates the behavior.

## Syntax

The current proposal introduces an optional, enum class label argument to the existing `__Memoize` (and `__MemoizeLSB`) attributes e.g. `__Memoize(#KeyedByIC)`. This would require adding the ability to use enum class labels in attribute argument positions.

```
enum class MemoizeOption: string {
  string KeyedByIC = 'KeyedByIC';
  string MakeICInaccessible = 'MakeICInaccessible';
  string SoftMakeICInaccessible = 'SoftMakeICInaccessible';
  string Uncategorized = 'Uncategorized';
}
```

Currently, the `__Memoize` attribute is hard-coded into the typechecker and there is no declaration for it. However, you could declare it like so:

```
final class __Memoize implements HH\FunctionAttribute, HH\MethodAttribute {
  public function __construct(
    private ?HH\EnumClass\Label<MemoizeOption, string> $kind,
  ) {}
}
```

Alternatives we considered:

* Using special constants e.g. `__Memoize(KeyedByIC)`. However, constants are not permitted as attribute arguments as attributes are evaluated at compile time and using evaluation constants would require “decls in compilation.” We also compute attributes in order to build indices for Facts which may never have access to cross-file declarations.
* Using special strings e.g. `__Memoize("KeyedByIC")` — this has the downside of a string argument appears to take arbitrary strings, but can only take specific, special strings. This may also require additional changes to support autocompleting these strings in IDEs.
* Creating a new attribute to replace each variant e.g. `__MemoizeKeyedByIC` and `__MemoizeMakeICInaccessible`
    * If we choose this option, we may consider more fluid names like `__MemoizeWithIC` and `__MemoizeAndMakeICInaccessible`
    * This has the downside of adding each attribute for memoize and lsb variants, increasing the code complexity in parser/runtime
* Creating auxiliary attributes to use alongside `__Memoize` e.g. `<<__Memoize, __MakeICInaccessible>>`
    * This option has the downside of introducing attributes that can only be used with other attributes which would be especially undesirable for `__Memoize(KeyedByIC)`
    * We could conceivably allow `__MakeICInaccessible` to be used standalone. However, at the moment, we do not see a legitimate use for it without an accompanying `__Memoize`.

## Naming

"KeyedByIC" uses "Keyed" which references the fact that the IC is incorporated into the key used in the memoization cache. We could also possibly use imperative "Key" vs past participle "Keyed." Possible alternatives to the verb "key:"

* "Shard" - original verbiage from "PolicyShardedMemoize" but noted possible incorrectly inferred association with database sharding
* "Split"
* "Partition"

"MakeICInaccessible" describes how the IC cannot be accessed. A previously proposed name used the verbiage "clear IC," however this created a distinction between a state where the IC was "cleared" vs one where the IC was never set. Primary concerns with this name are aesthetic: "inaccessible" is long and may be awkward to say and spell. A name like <imperative verb> + "IC" may be preferable, but verbs like "prohibit" or "ban" may incorrectly imply immediate exceptional behavior of the function with the attribute instead of restrictions on the function's dependencies.

"Soft" has analogous meaning in the `__Soft` attribute for typehints where behavior that would normally throw exceptions results in logs instead which can be used to eventually move code to the "hard" throwing behavior.

“IC” as an abbreviation for “Implicit Context” was chosen for brevity and lack of more attractive options. We concluded that brevity was valuable given expected prevalent usage of these attributes and that the disadvantage of possible ambiguity of “IC” would be offset by the fact that the full symbols names would be sufficiently unique. Other alternatives that were considered:

* “IArg” as in “Implicit Argument” — This primary issue with this option is that “implicit arguments” is a feature in other languages with a different meaning. Using this term would both set incorrect expectations for readers with experience in those languages as well as be problematic should we want Hack to adopt a similar feature.
* “ICtx” (using “ctx” as an alternative abbreviation for “context”) — The primary objection to this was that while we use “ctx” as an abbreviation for “context” in other features of the language, in those cases, “ctx” refers to a coeffects “context,” which, while there are some tie-ins, the “contexts” in each case are distinct concepts.
* “ImplicitContext” — This was considered too long, especially in `__Memoize(#SoftMakeImplicitContextInaccessible)`.
* “Ctx” or “Context” (without “implicit”) — This was considered insufficiently disambiguated as “context” is already an overloaded name. A developer may reasonably, but incorrectly, assume that “KeyedByContext” would mean “keyed by the coeffects context.”
* Other names: “Implicit Value” or “IVal,” “ScopedValue,” “ContextValue” or “CVal” or “CtxVal”

## Multiple Implicit Contexts

The current implementation of Implicit Contexts allows for multiple IC “flavors.” You create a flavor by defining a class implementing the native `HH\ImplicitContext` class. This presents the question for how the above features should work when there is more than one IC flavor.

We have identified a few options:

1. **Only allow a single flavor and require any further division of the value to be done in userland.** This has the benefit of simplifying the semantics. This also inherently requires coupling among all flavors, but unless you parameterize `KeyedByIC` and `MakeICInaccessible` features by flavor, the flavors are already tightly coupled since they will need to agree on when these attributes ought to be used. Coupling is also introduced by sharing related coeffects and the backdoor functions.
2. **Track an IC state per flavor and allow individual setting.** Each flavor’s state could be set individual by `runWith` and `soft_run_with` calls, but `MakeICInaccesible` would set all flavors’ state to `inaccessible`. This would likely require more checks at runtime resulting in worsened performance.
3. **Only track flavors in the value state.** Setting one flavor from the `inaccessible` state would resulting in implicitly setting all other flavors’ value to the initial null state.

Our recommendation is that we take Option 3 which is closest to the current implementation of Implicit Contexts but require in userland that there is at most a single child of `HH\ImplicitContext`. This effectively means choosing Option 1 from the perspective of the user. This compromise allows us to make fewer changes to the runtime in the near term. We can choose to simplify the runtime to not support multiple flavors at a later date.
