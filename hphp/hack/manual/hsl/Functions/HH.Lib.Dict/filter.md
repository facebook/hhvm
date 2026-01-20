
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict containing only the values for which the given predicate
returns ` true `




``` Hack
namespace HH\Lib\Dict;

function filter<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
  ?(function(Tv): bool) $value_predicate = NULL,
): dict<Tk, Tv>;
```




The default predicate is casting the value to boolean.




+ To remove null values in a typechecker-visible way, see ` Dict\filter_nulls() `.
+ To use an async predicate, see ` Dict\filter_async() `.




Time complexity: O(n * p), where p is the complexity of ` $value_predicate `
(which is O(1) if not provided explicitly)
Space complexity: O(n)




## Parameters




* [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $traversable ``
* ` ?(function(Tv): bool) $value_predicate = NULL `




## Returns




- ` dict<Tk, Tv> `
<!-- HHAPIDOC -->
