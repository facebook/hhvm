
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Set `](/apis/Classes/HH/Set/) containing the values of the current [` Set `](/apis/Classes/HH/Set/) starting after
and including the first value that produces `` true `` when passed to the
specified callback




``` Hack
public function skipWhile(
  (function(Tv): bool) $fn,
): Set<Tv>;
```




The returned [` Set `](/apis/Classes/HH/Set/) will always be a proper subset of the current [` Set `](/apis/Classes/HH/Set/).




## Parameters




+ ` (function(Tv): bool) $fn ` - The callback used to determine the starting element for the
  [` Set `](/apis/Classes/HH/Set/).




## Returns




* [` Set<Tv> `](/apis/Classes/HH/Set/) - A [` Set `](/apis/Classes/HH/Set/) that is a proper subset of the current [` Set `](/apis/Classes/HH/Set/) starting
  after the callback returns `` true ``.




## Examples




This example shows how ` skipWhile ` can be used to create a new [` Set `](/apis/Classes/HH/Set/) by skipping elements at the beginning of an existing [` Set `](/apis/Classes/HH/Set/):




``` basic-usage.hack
$s = Set {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144};

// Skip values until we reach one over 10
$s2 = $s->skipWhile($x ==> $x <= 10);
\var_dump($s2);
```
<!-- HHAPIDOC -->
