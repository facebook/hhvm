
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Like ` filter_async `, but lets you utilize the keys of your dict too




``` Hack
namespace HH\Lib\Dict;

function filter_with_key_async<Tk as arraykey, Tv>(
  KeyedContainer<Tk, Tv> $traversable,
  (function(Tk, Tv): Awaitable<bool>) $predicate,
): Awaitable<dict<Tk, Tv>>;
```




For non-async filters with key, see ` Dict\filter_with_key() `.




Time complexity: O(n * p), where p is the complexity of ` $value_predicate `
Space complexity: O(n)




## Parameters




+ [` KeyedContainer<Tk, `](/apis/Interfaces/HH/KeyedContainer/)`` Tv> $traversable ``
+ ` (function(Tk, Tv): Awaitable<bool>) $predicate `




## Returns




* [` Awaitable<dict<Tk, `](/apis/Classes/HH/Awaitable/)`` Tv>> ``
<!-- HHAPIDOC -->
