
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Set `](/apis/Classes/HH/Set/) containing the values of the current [` Set `](/apis/Classes/HH/Set/) up to but not
including the first value that produces `` false `` when passed to the
specified callback




``` Hack
public function takeWhile(
  (function(Tv): bool) $callback,
): Set<Tv>;
```




The returned [` Set `](/apis/Classes/HH/Set/) will always be a proper subset of the current [` Set `](/apis/Classes/HH/Set/).




## Parameters




+ ` (function(Tv): bool) $callback `




## Returns




* [` Set<Tv> `](/apis/Classes/HH/Set/) - A [` Set `](/apis/Classes/HH/Set/) that is a proper subset of the current [` Set `](/apis/Classes/HH/Set/) up until
  the callback returns `` false ``.




## Examples




This example shows how ` takeWhile ` can be used to create a new [` Set `](/apis/Classes/HH/Set/) by taking elements from the beginning of an existing [` Set `](/apis/Classes/HH/Set/):




``` basic-usage.hack
$s = Set {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144};

// Include values until we reach one over 10
$s2 = $s->takeWhile($x ==> $x <= 10);
\var_dump($s2);
```
<!-- HHAPIDOC -->
