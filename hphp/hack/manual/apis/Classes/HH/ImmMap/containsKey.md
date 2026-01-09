
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Determines if the specified key is in the current [` ImmMap `](/docs/apis/Classes/HH/ImmMap/)




``` Hack
public function containsKey(
  mixed $key,
): bool;
```




This function is interchangeable with [` contains() `](/docs/apis/Classes/HH/ImmMap/contains/).




## Guide




+ [Constraints](</docs/hack/generics/type-constraints>)







## Parameters




* ` mixed $key `




## Returns




- ` bool ` - `` true `` if the specified key is present in the current [` ImmMap `](/docs/apis/Classes/HH/ImmMap/);
  `` false `` otherwise.




## Examples




See [`Map::containsKey`](/docs/apis/Classes/HH/Map/containsKey/#examples) for usage examples.
<!-- HHAPIDOC -->
