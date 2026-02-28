
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Determines if the specified key is in the current [` Map `](/apis/Classes/HH/Map/)




``` Hack
public function contains(
  mixed $key,
): bool;
```




This function is interchangeable with [` containsKey() `](/apis/Classes/HH/Map/containsKey/).




## Guide




+ [Constraints](</hack/generics/type-constraints>)







## Parameters




* ` mixed $key `




## Returns




- ` bool ` - `` true `` if the specified key is present in the current [` Map `](/apis/Classes/HH/Map/);
  returns `` false `` otherwise.




## Examples




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
};

// Prints "true", since key "red" is the first key
\var_dump($m->containsKey('red'));

// Prints "true", since key "yellow" is the last key
\var_dump($m->containsKey('yellow'));

// Prints "false", since key "blurple" isn't in the Map
\var_dump($m->containsKey('blurple'));
```
<!-- HHAPIDOC -->
