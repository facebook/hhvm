
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict containing only the values for which the given async
predicate returns ` true `




``` Hack
namespace HH\Lib\Dict;

function filter_async<Tk as arraykey, Tv>(
  KeyedContainer<Tk, Tv> $traversable,
  (function(Tv): Awaitable<bool>) $value_predicate,
): Awaitable<dict<Tk, Tv>>;
```




For non-async predicates, see ` Dict\filter() `.




Time complexity: O(n * p), where p is the complexity of the synchronous
portions of ` $value_predicate `
Space complexity: O(n)




The IO operations for each of the calls to ` $value_predicate ` will happen
in parallel.




## Parameters




+ [` KeyedContainer<Tk, `](/apis/Interfaces/HH/KeyedContainer/)`` Tv> $traversable ``
+ ` (function(Tv): Awaitable<bool>) $value_predicate `




## Returns




* [` Awaitable<dict<Tk, `](/apis/Classes/HH/Awaitable/)`` Tv>> ``
<!-- HHAPIDOC -->
