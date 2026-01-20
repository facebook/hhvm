
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new vec containing only the values for which the given async
predicate returns ` true `




``` Hack
namespace HH\Lib\Vec;

function filter_async<Tv>(
  Container<Tv> $container,
  (function(Tv): Awaitable<bool>) $value_predicate,
): Awaitable<vec<Tv>>;
```




For non-async predicates, see ` Vec\filter() `.




Time complexity: O(n * p), where p is the complexity of synchronous portions
of ` $value_predicate `
Space complexity: O(n)




The IO operations for each of the calls to ` $value_predicate ` will happen
in parallel.




## Parameters




+ [` Container<Tv> `](/apis/Interfaces/HH/Container/)`` $container ``
+ ` (function(Tv): Awaitable<bool>) $value_predicate `




## Returns




* [` Awaitable<vec<Tv>> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
