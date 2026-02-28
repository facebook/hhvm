
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the index of the first element that matches the search value




``` Hack
public function linearSearch<Tu super mixed>(
  mixed $search_value,
): int;
```




If no element matches the search value, this function returns -1.




## Guide




+ [Constraints](</hack/generics/type-constraints>)







## Parameters




* ` mixed $search_value ` - The value that will be search for in the current
  [` Pair `](/apis/Classes/HH/Pair/).




## Returns




- ` int ` - The key (index) where that value is found; -1 if it is not found.




## Examples




``` basic-usage.hack
$p = Pair {'foo', -1.5};

// Prints 0 (the index of the first value)
\var_dump($p->linearSearch('foo'));

// Prints 1 (the index of the second value)
\var_dump($p->linearSearch(-1.5));

// Prints -1 (the value doesn't exist in the Pair)
\var_dump($p->linearSearch('bar'));
```
<!-- HHAPIDOC -->
