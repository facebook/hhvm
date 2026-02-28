
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Vector `](/apis/Classes/HH/Vector/) containing the values of the current [` Vector `](/apis/Classes/HH/Vector/) that meet
a supplied condition




``` Hack
public function filter(
  (function(Tv): bool) $callback,
): Vector<Tv>;
```




[` filter() `](/apis/Classes/HH/Vector/filter/)'s result contains only values that meet the provided criterion;
unlike [` map() `](/apis/Classes/HH/Vector/map/), where a value is included for each value in the original
[` Vector `](/apis/Classes/HH/Vector/).




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): bool) $callback `




## Returns




- [` Vector<Tv> `](/apis/Classes/HH/Vector/) - A [` Vector `](/apis/Classes/HH/Vector/) containing the values after a user-specified condition
  is applied.




## Examples




``` basic-usage.hack
$colors = Vector {'red', 'green', 'blue', 'yellow'};
$primary_colors = Set {'red', 'green', 'blue'};

// Create a Vector of colors that contain the letter 'l'
$l_colors = $colors->filter($color ==> \strpos($color, 'l') !== false);
\var_dump($l_colors);

// Create a Vector of colors that aren't listed in $primary_colors
$non_primary_colors = $colors->filter(
  $color ==> !$primary_colors->contains($color),
);
\var_dump($non_primary_colors);
```
<!-- HHAPIDOC -->
