
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Set `](/docs/apis/Classes/HH/Set/) containing the values of the current [` Set `](/docs/apis/Classes/HH/Set/) that meet
a supplied condition applied to each value




``` Hack
public function filter(
  (function(Tv): bool) $callback,
): Set<Tv>;
```




Only values that meet a certain criteria are affected by a call to
[` filter() `](/docs/apis/Classes/HH/Set/filter/), while all values are affected by a call to [` map() `](/docs/apis/Classes/HH/Set/map/).




## Guide




+ [Examples](</docs/hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): bool) $callback `




## Returns




- [` Set<Tv> `](/docs/apis/Classes/HH/Set/) - a [` Set `](/docs/apis/Classes/HH/Set/) containing the values after a user-specified condition
  is applied.




## Examples




``` basic-usage.hack
$colors = Set {'red', 'green', 'blue', 'yellow'};

// Create a Set of colors that contain the letter 'l'
$l_colors = $colors->filter($color ==> \strpos($color, 'l') !== false);
\var_dump($l_colors);
```
<!-- HHAPIDOC -->
