
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Map `](/apis/Classes/HH/Map/) containing the values of the current [` Map `](/apis/Classes/HH/Map/) that meet
a supplied condition




``` Hack
public function filter(
  (function(Tv): bool) $callback,
): Map<Tk, Tv>;
```




Only values that meet a certain criteria are affected by a call to
[` filter() `](/apis/Classes/HH/Map/filter/), while all values are affected by a call to [` map() `](/apis/Classes/HH/Map/map/).




The keys associated with the current [` Map `](/apis/Classes/HH/Map/) remain unchanged in the returned
[` Map `](/apis/Classes/HH/Map/).




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): bool) $callback `




## Returns




- [` Map<Tk, `](/apis/Classes/HH/Map/)`` Tv> `` - a [` Map `](/apis/Classes/HH/Map/) containing the values after a user-specified condition
  is applied.




## Examples




This example shows how ` filter ` returns a new [` Map `](/apis/Classes/HH/Map/) containing only the values (and their corresponding keys) for which `` $callback `` returned ``` true ```:




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
  'purple' => '#663399',
};

// Filter $m for colors with a 100% red component
$red_100 = $m->filter($hex_code ==> \strpos($hex_code, '#ff') === 0);
\var_dump($red_100);
```
<!-- HHAPIDOC -->
