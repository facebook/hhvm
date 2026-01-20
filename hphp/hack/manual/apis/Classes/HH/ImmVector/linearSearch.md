
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the index of the first element that matches the search value




``` Hack
public function linearSearch(
  mixed $search_value,
): int;
```




If no element matches the search value, this function returns -1.




## Guide




+ [Constraints](</hack/generics/type-constraints>)







## Parameters




* ` mixed $search_value ` - The value that will be searched for in the current
  [` ImmVector `](/apis/Classes/HH/ImmVector/).




## Returns




- ` int ` - The key (index) where that value is found; -1 if it is not found.




## Examples




See [` Vector::linearSearch `](/apis/Classes/HH/Vector/linearSearch/#examples) for usage examples.
<!-- HHAPIDOC -->
