---
slug: map
---

:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Map `](/apis/Classes/HH/Map/) after an operation has been applied to each value in the
current [` Map `](/apis/Classes/HH/Map/)




``` Hack
public function map<Tu>(
  (function(Tv): Tu) $callback,
): Map<Tk, Tu>;
```




Every value in the current [` Map `](/apis/Classes/HH/Map/) is affected by a call to [` map() `](/apis/Classes/HH/Map/map/), unlike
[` filter() `](/apis/Classes/HH/Map/filter/) where only values that meet a certain criteria are affected.




The keys will remain unchanged from the current [` Map `](/apis/Classes/HH/Map/) to the returned
[` Map `](/apis/Classes/HH/Map/).




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): Tu) $callback `




## Returns




- [` Map<Tk, `](/apis/Classes/HH/Map/)`` Tu> `` - a [` Map `](/apis/Classes/HH/Map/) containing key/value pairs after a user-specified
  operation is applied.




## Examples




In this example the [` Map `](/apis/Classes/HH/Map/)'s values are mapped to the same type (`` string ``s):




``` map-to-strings.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
};

$capitalized = $m->map(fun('strtoupper'));
\var_dump($capitalized);

$css_colors = $capitalized->map($hex_code ==> "color: {$hex_code};");
\var_dump($css_colors);
```




In this example the [` Map `](/apis/Classes/HH/Map/)'s values are mapped to a different type (`` int ``s):




``` map-to-ints.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
};

$decimal_codes = $m->map(fun('hexdec'));
\var_dump($decimal_codes);
```
<!-- HHAPIDOC -->
