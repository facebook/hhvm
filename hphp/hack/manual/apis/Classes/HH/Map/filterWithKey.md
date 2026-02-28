
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Map `](/apis/Classes/HH/Map/) containing the values of the current [` Map `](/apis/Classes/HH/Map/) that meet
a supplied condition applied to its keys and values




``` Hack
public function filterWithKey(
  (function(Tk, Tv): bool) $callback,
): Map<Tk, Tv>;
```




Only keys and values that meet a certain criteria are affected by a call to
[` filterWithKey() `](/apis/Classes/HH/Map/filterWithKey/), while all values are affected by a call to
[` mapWithKey() `](/apis/Classes/HH/Map/mapWithKey/).




The keys associated with the current [` Map `](/apis/Classes/HH/Map/) remain unchanged in the
returned [` Map `](/apis/Classes/HH/Map/); the keys will be used in the filtering process only.




## Parameters




+ ` (function(Tk, Tv): bool) $callback `




## Returns




* [` Map<Tk, `](/apis/Classes/HH/Map/)`` Tv> `` - a [` Map `](/apis/Classes/HH/Map/) containing the values after a user-specified condition
  is applied to the keys and values of the current [` Map `](/apis/Classes/HH/Map/).




## Examples




This example shows how ` filterWithKey ` allows you to use an element's value and corresponding key to determine whether to include it in the filtered [` Map `](/apis/Classes/HH/Map/).




``` basic-usage.hack
$m = Map {
  'red' => '#ff0000',
  'green' => '#00ff00',
  'blue' => '#0000ff',
  'yellow' => '#ffff00',
  'purple' => '#663399',
};

$primary_colors = Set {'red', 'green', 'blue'};

// Filter non-primary colors with a 100% red component
$non_primary_red_100 = $m->filterWithKey(
  ($color, $hex_code) ==> {
    $is_primary = $primary_colors->contains($color);
    $has_full_red_component = \strpos($hex_code, '#ff') === 0;
    return $has_full_red_component && !$is_primary;
  },
);

\var_dump($non_primary_red_100);
```
<!-- HHAPIDOC -->
