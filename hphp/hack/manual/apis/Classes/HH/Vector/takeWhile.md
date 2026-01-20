
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Vector `](/apis/Classes/HH/Vector/) containing the values of the current [` Vector `](/apis/Classes/HH/Vector/) up to but
not including the first value that produces `` false `` when passed to the
specified callback




``` Hack
public function takeWhile(
  (function(Tv): bool) $callback,
): Vector<Tv>;
```




That is, takes the continuous prefix of values in
the current [` Vector `](/apis/Classes/HH/Vector/) for which the specified callback returns `` true ``.




The returned [` Vector `](/apis/Classes/HH/Vector/) will always be a subset (but not necessarily a
proper subset) of the current [` Vector `](/apis/Classes/HH/Vector/).




## Parameters




+ ` (function(Tv): bool) $callback `




## Returns




* [` Vector<Tv> `](/apis/Classes/HH/Vector/) - A [` Vector `](/apis/Classes/HH/Vector/) that is a subset of the current [` Vector `](/apis/Classes/HH/Vector/) up until the
  callback returns `` false ``.




## Examples




This example shows how ` takeWhile ` can be used to create a new [` Vector `](/apis/Classes/HH/Vector/) by taking elements from the beginning of an existing [` Vector `](/apis/Classes/HH/Vector/):




``` basic-usage.hack
$v = Vector {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144};

// Include values until we reach one over 10
$v2 = $v->takeWhile($x ==> $x <= 10);
\var_dump($v2);
```
<!-- HHAPIDOC -->
