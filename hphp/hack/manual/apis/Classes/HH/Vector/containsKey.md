
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Determines if the specified key is in the current [` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function containsKey(
  mixed $key,
): bool;
```




## Guide




+ [Constraints](</hack/generics/type-constraints>)







## Parameters




* ` mixed $key `




## Returns




- ` bool ` - `` true `` if the specified key is present in the current [` Vector `](/apis/Classes/HH/Vector/);
  returns `` false `` otherwise.




## Examples




``` basic-usage.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};

// Prints "true", since index 0 is the first element
\var_dump($v->containsKey(0));

// Prints "true", since index 3 is the last element
\var_dump($v->containsKey(3));

// Prints "false", since index 10 doesn't exist
\var_dump($v->containsKey(10));
```
<!-- HHAPIDOC -->
