
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Map `](/apis/Classes/HH/Map/) after an operation has been applied to each key and
value in the current [` Map `](/apis/Classes/HH/Map/)




``` Hack
public function mapWithKey<Tu>(
  (function(Tk, Tv): Tu) $callback,
): Map<Tk, Tu>;
```




Every key and value in the current [` Map `](/apis/Classes/HH/Map/) is affected by a call to
[` mapWithKey() `](/apis/Classes/HH/Map/mapWithKey/), unlike [` filterWithKey() `](/apis/Classes/HH/Map/filterWithKey/) where only values that meet a
certain criteria are affected.




The keys will remain unchanged from the current [` Map `](/apis/Classes/HH/Map/) to the returned
[` Map `](/apis/Classes/HH/Map/). The keys are only used to help in the mapping operation.




## Parameters




+ ` (function(Tk, Tv): Tu) $callback `




## Returns




* [` Map<Tk, `](/apis/Classes/HH/Map/)`` Tu> `` - a [` Map `](/apis/Classes/HH/Map/) containing the values after a user-specified operation
  on the current [` Map `](/apis/Classes/HH/Map/)'s keys and values is applied.




## Examples




This example shows how ` mapWithKey ` can be used to create a new [` Map `](/apis/Classes/HH/Map/) based on `` $m ``'s keys and values:




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
};

$css_colors = $m->mapWithKey(
  ($color, $hex_code) ==> "color: {$hex_code}; /* {$color} */",
);

echo \implode("\n", $css_colors)."\n";
```
<!-- HHAPIDOC -->
