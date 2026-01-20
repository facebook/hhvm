
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmSet `](/apis/Classes/HH/ImmSet/) containing the values of the current [` ImmSet `](/apis/Classes/HH/ImmSet/) up to
but not including the first value that produces `` false `` when passed to the
specified callback




``` Hack
public function takeWhile(
  (function(Tv): bool) $callback,
): ImmSet<Tv>;
```




The returned [` ImmSet `](/apis/Classes/HH/ImmSet/) will always be a proper subset of the current
[` ImmSet `](/apis/Classes/HH/ImmSet/).




## Parameters




+ ` (function(Tv): bool) $callback `




## Returns




* [` ImmSet<Tv> `](/apis/Classes/HH/ImmSet/) - An [` ImmSet `](/apis/Classes/HH/ImmSet/) that is a proper subset of the current [` ImmSet `](/apis/Classes/HH/ImmSet/) up
  until the callback returns `` false ``.




## Examples




See [` Set::takeWhile `](/apis/Classes/HH/Set/takeWhile/#examples) for usage examples.
<!-- HHAPIDOC -->
