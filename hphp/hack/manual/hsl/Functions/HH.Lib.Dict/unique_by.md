
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict in which each value appears exactly once, where the
value's uniqueness is determined by transforming it to a scalar via the
given function




``` Hack
namespace HH\Lib\Dict;

function unique_by<Tk as arraykey, Tv, Ts as arraykey>(
  KeyedContainer<Tk, Tv> $container,
  (function(Tv): Ts) $scalar_func,
): dict<Tk, Tv>;
```




In case of duplicate scalar values, later keys will overwrite
the previous ones.




For arraykey values, see ` Dict\unique() `.




Time complexity: O(n * s), where s is the complexity of ` $scalar_func `
Space complexity: O(n)




## Parameters




+ [` KeyedContainer<Tk, `](/apis/Interfaces/HH/KeyedContainer/)`` Tv> $container ``
+ ` (function(Tv): Ts) $scalar_func `




## Returns




* ` dict<Tk, Tv> `
<!-- HHAPIDOC -->
