```yamlmeta
{
  "fbonly messages": [
    "There is [an internal wiki](https://www.internalfb.com/intern/wiki/Hack_Foundation/Memoization_options/) with more, Meta-specific guidance."
  ]
}
```

The `__Memoize` and `__MemoizeLSB` attributes in Hack support experimental options that control how the function behaves with respect to an Implicit Context value.

## Introduction

The Implicit Context (IC) is an experimental feature. It is a value that can be associated with the execution of code and propagated implicitly from caller to callee in a way that is safe for [concurrently running code](/hack/asynchronous-operations/introduction).

In order to prevent the problem of "memoization cache poisoning" where results computed using one IC value are available to callers using a different IC value, we require that memoized functions executed with an IC value are explicit about how they should behave relative to the IC value. Broadly, functions can specify one of two choices:

* This function's memoization cache key _should_ incorporate the propagated IC value.
* This function's memoization cache key _should not_ incorporate the propagated IC value *AND* this function and its dependencies cannot depend on the propagated IC value.

These requirements are enforced at runtime via thrown exceptions and not statically by the typechecker.
If static requirements are desired, one can make use of special [contexts and capabilities](/hack/contexts-and-capabilities/introduction) specifically `zoned`.

## Available Options

A memoized function may specify its behavior relative to the IC by passing an option to the `__Memoize` (or `__MemoizeLSB`) attribute.

```Hack
<<__Memoize(#MakeICInaccessible)>>
function get_same_random_int(): int {
  return HH\Lib\PseudoRandom\int();
}
```

There are two options that are compatible with propagating an IC value — `#KeyedByIC` and `#MakeICInaccessible` — and two options that are not — `#SoftMakeICInaccessible` and `#Uncategorized`. Functions with one of the former pair of options are considered “categorized” while functions with one of the latter pair of options is considered “uncategorized.” See below for the treatment of memoized functions that do not pass an argument to the memoize attribute.

* `#KeyedByIC` indicates that the function’s memoization cache should include the IC value in the cache key.
    * Memoized functions that depend on the IC should use this option.
    * Memoized functions with the `[zoned]` context must use this option.
    * A function must have one of the following contexts to use this option: `defaults` (implicitly or explicitly), `zoned`, `zoned_local`, `zoned_shallow`
    * Note that the cache may also be keyed on metadata necessary for sufficiently complete migration logging and so you should not assume that just because your function isn’t currently being called with a propagated IC value that it is only called once.
* `#MakeICInaccessible` indicates that the function’s memoization cache should *not* include the IC value in the cache key and that the function does not access the IC.
    * Any direct or indirect calls to the builtin function that fetches the IC will result in an exception.
    * Any direct or indirect calls to an uncategorized memoized function will throw.
    * A function must have one of the following contexts to use this option: `defaults` (implicitly or explicitly), `leak_safe_shallow`, `leak_safe_local`
* `#SoftMakeICInaccessible` is a migration option for functions that are intended to be marked with `#MakeICInaccessible`.
    * The function’s memoization cache will *not* include the IC value in the cache key.
    * In circumstances in which using `#MakeICInaccessible` will throw, using `#SoftMakeICInaccessible` instead will result in runtime logs.
    * Note that functions using this operation are still considered “uncategorized.”
* `#Uncategorized` is an option that explicitly indicates that a function has not yet described its intended behavior relative to the IC.
* If no option is provided, the function’s memoization cache will not include the IC value, but whether or not it is considered “categorized” or “uncategorized” will depend on the function’s context list.
    * If the functions’ context already prevents the function from fetching the IC which requires the `[zoned]` context (i.e. the function is as capable as or less capable than `[leak_safe, globals]`) then no option is allowed and the function is considered categorized.
    * Otherwise, the function is considered uncategorized.

>Note that this syntax uses “[enum class labels](/hack/built-in-types/enum-class-label)” as arguments. Currently, only the memoization attributes are allowed to use labels as arguments.

## Related classes and functions

```yamlmeta
{
  "fbonly messages": [
    "Meta developers should not use these symbols directly and instead should only use them via Zones APIs."
  ]
}
```

* Executing code with an IC value and accessing the propagated value is done by extending and using methods of the class `HH\ImplicitContext`.
* Functions `HH\ImplicitContext\soft_run_with` and `HH\ImplicitContext\soft_run_with_async` are used to execute code in a migratory state ahead of using `HH\ImplicitContext::runWith` or `HH\ImplicitContext::runWithAsync` -- in this migratory state, calls to uncategorized memoized functions produce runtime warnings instead of exceptions.
