Title: Equality, Identity, and Comparisons

Start Date: June 24 2020

Status: Draft

## Summary

Unify `==` and `===`, introduce sane alternatives for classes that need to use
behaviour similar to the old versions.

## Feature motivation

Hack has two main comparison operators `==` and `===`. They're individually
comprised of behaviours both good and bad, and combine to a muddied, confusing,
and overall broken experience. `==` does coercing value equality, including on
objects, and compares arrays (sometimes unorderedly). `===` does pointer equality
checks on objects (including collections), and breaks some invariants for
reactive and pure code.

We'd like to simplify and correct the mistakes of the previous design

### The current state

1. `===`
   1. Objects: pointer equality (note that closures are instances of different objects)
   2. arrays: compares all elements (keys and values) using `===`, requiring identical internal ordering.
   3. values: value equality
   4. Hack Collections: pointer equality (they are objects)
2. `==`
   1. Objects: compares all properties using `==`
     1. note that closures still don’t compare equal because they’re instances of different objects
   2. arrays: compares all elements (keys and values) using `===` for keys and `==` for values, ignoring order for `dicts`/`darrays`/`keysets`
   3. values: value equality following complex PHP coercion rules.
   4. Hack Collections: compares all elements using `==`,  ignoring order for `Map`/`Set`
   5. If comparing different types will attempt to coerce one side to the type of the other before comparing
3. `<=`, `>=`: Use `==` under the hood. No `===` variant.
4. Sorts and switch statements implicitly use the == behaviour under the hood

## User experience

1. To test object pointer equality, use builtin `is_same_obj($a, $b)` that does a pointer check
   1. Potentially would require a marker interface such as `IIdentityTestable` (name to be bikeshed).
   2. In that case, whether `is_same_obj` is free standing or works as `$a→isSame($b)` to be bikeshed
      1. In the method case, this is non-overrideable
2. To get object structural equality require explicit opt-in via an interface such as `IEquatable`
   1. To be bikeshed: does this allow using `==` or does it do something like `$a→eq($b)`
      1. In the method case, this is non-overrideable
   2. Would require all properties to be either values or also `IEquatable`
   3. Note that this would probably do an identity test first as an optimization
      1. This also would require reflexivity for the optimization to be sound.
   4. If we can’t compare Collections and Arrays, how do props of those types work? Do we allow it implicitly in this case?
3. For the previous two, if we go the object method route of comparison, absolutely cannot allow overriding implementations.
4. `==` works as follows
   1. never coerces the arguments
   2. Objects: see 1 and 2.
      1. If `IEquatable` means can use `==`, do we return false or throw when not present?
      2. What happens when objects have different types?
   3. Bikeshed more: Cannot compare arrays/collections. Use HSL or Collections methods
   4. Values: work the same
   5. Closures are not `IEquatable`, and pointer equality on two of them is always false (or throws?).
      1. This will cause issues with reflexivity.
   6. bikeshed: `resource` and `NaN`? Other edge cases?
      1. Note that NaN is already weird. Currently `NaN !== Nan` but `vec[NAN] === vec[NAN]`
5. `===` doesn’t exist
6. `<=`,`>=`, etc inherit the changes to ==
   1. Are `IEquatable` objects comparable this way? I expect not.
   2. Do we allow them on Containers?
7. Having an object-arg to a memoized `pure`/`rx` function requires them to be `IEquatable`
8. Sorts and switch statements use the new sane == under the hood
9. Will likely need a builtin coercing_eq_yikity_yikes() that does the old == behaviour for migration purposes

## IDE experience

N/A

## Implementation details

Mostly still open questions above or below. Did I miss any?

What the typing rules of == under this proposal? Is it still a function that takes two values of type mixed?

## Design rationale and alternatives

TODO once we get more consensus about the open questions above

## Drawbacks

TODO

It's a ton of work. Probably not HAM level, but definitely a major project from the HF perspectivce

## Prior art

TODO

Note rust's ord, eq, partial ord, and partial eq. Rust (which took these ideas from Haskell) only allows you to compare values that implement these traits (typeclasses in Haskell).

## Unresolved Questions (in addition to the currently inline ones above)

How do function pointers work? And are you guaranteed to get the same pointer for two different calls

Does structural equality on well-formed IEquatable classes always work?

```
class ListNode implements IEquatable {
  public function __construct(
    public int $val,
    public ?ListNode $prev = null,
    public ?ListNode $next = null,
  ) {}
}

function compare_nodes(): void {
  // Build a linked list [1, 2].
  $n = new ListNode(1);
  $n2 = new ListNode(2, $n);
  $n->next = $n2;

  // What value does $b have?
  $b = $n == $n2;
}
```
The obvious recursive function for structural equality would loop infinitely on $n == $n2.


## Future possibilities

I think this is mostly N/A? Unless we want to reuse === for something?
