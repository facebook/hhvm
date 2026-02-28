
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict where each mapping is defined by the given key/value
tuples




``` Hack
namespace HH\Lib\Dict;

function from_entries<Tk as arraykey, Tv>(
  Traversable<(Tk, Tv)> $entries,
): dict<Tk, Tv>;
```




In the case of duplicate keys, later values will overwrite the
previous ones.




+ To create a dict from keys, see ` Dict\from_keys() `.
+ To create a dict from values, see ` Dict\from_values() `.




Also known as ` unzip ` or `` fromItems `` in other implementations.




Time complexity: O(n)
Space complexity: O(n)




## Parameters




* [` Traversable<(Tk, `](/apis/Interfaces/HH/Traversable/)`` Tv)> $entries ``




## Returns




- ` dict<Tk, Tv> `
<!-- HHAPIDOC -->
