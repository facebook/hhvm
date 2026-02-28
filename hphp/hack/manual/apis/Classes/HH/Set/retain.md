
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Alters the current [` Set `](/apis/Classes/HH/Set/) so that it only contains the values that meet a
supplied condition on each value




``` Hack
public function retain(
  (function(Tv): bool) $callback,
): Set<Tv>;
```




This method is like [` filter() `](/apis/Classes/HH/Set/filter/), but mutates the current [` Set `](/apis/Classes/HH/Set/) too in
addition to returning the current [` Set `](/apis/Classes/HH/Set/).




Future changes made to the current [` Set `](/apis/Classes/HH/Set/) ARE reflected in the returned
[` Set `](/apis/Classes/HH/Set/), and vice-versa.




## Parameters




+ ` (function(Tv): bool) $callback `




## Returns




* [` Set<Tv> `](/apis/Classes/HH/Set/) - Returns itself.




## Examples




``` basic-usage.hack
$s = Set {'red', 'green', 'blue', 'yellow'};

// Only keep values beginning with 'r' or 'b'
$s->retain($color ==> $color[0] === 'r' || $color[0] === 'b');
\var_dump($s);
```
<!-- HHAPIDOC -->
