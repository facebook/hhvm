
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Checks whether a provided key exists in the current [` Pair `](/apis/Classes/HH/Pair/)




``` Hack
public function containsKey<Tu super int>(
  Tu $key,
): bool;
```




This will only return ` true ` for provided keys of 0 and 1 since those are
the only two keys that can exist in a [` Pair `](/apis/Classes/HH/Pair/).




## Parameters




+ ` Tu $key `




## Returns




* ` bool ` - `` true `` if the provided key exists in the [` Pair `](/apis/Classes/HH/Pair/); `` false ``
  otherwise. This will only return ``` true ``` if the provided key is
  0 or 1.




## Examples




``` basic-usage.hack
$p = Pair {'foo', -1.5};

// Prints "true", since index 0 is the first value
\var_dump($p->containsKey(0));

// Prints "true", since index 1 is the second value
\var_dump($p->containsKey(1));

// Prints "false", since a Pair only has index 0 and index 1
\var_dump($p->containsKey(2));
```
<!-- HHAPIDOC -->
