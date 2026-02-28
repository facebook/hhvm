
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Determines if the specified key is in the current [` ImmMap `](/apis/Classes/HH/ImmMap/)




``` Hack
public function contains(
  mixed $key,
): bool;
```




This function is interchangeable with [` containsKey() `](/apis/Classes/HH/ImmMap/containsKey/).




## Guide




+ [Constraints](</hack/generics/type-constraints>)







## Parameters




* ` mixed $key `




## Returns




- ` bool ` - `` true `` if the specified key is present in the current [` ImmMap `](/apis/Classes/HH/ImmMap/);
  `` false `` otherwise.




## Examples




See [`Map::contains`](/apis/Classes/HH/Map/contains/#examples) for usage examples.
<!-- HHAPIDOC -->
