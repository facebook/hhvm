
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values of the current [` ImmVector `](/apis/Classes/HH/ImmVector/)
starting after and including the first value that produces `` false `` when
passed to the specified callback




``` Hack
public function skipWhile(
  (function(Tv): bool) $fn,
): ImmVector<Tv>;
```




That is, skips the continuous prefix of
values in the current [` ImmVector `](/apis/Classes/HH/ImmVector/) for which the specified callback returns
`` true ``.




The returned [` ImmVector `](/apis/Classes/HH/ImmVector/) will always be a subset (but not necessarily a
proper subset) of the current [` ImmVector `](/apis/Classes/HH/ImmVector/).




## Parameters




+ ` (function(Tv): bool) $fn ` - The callback used to determine the starting element for the
  returned [` ImmVector `](/apis/Classes/HH/ImmVector/).




## Returns




* [` ImmVector<Tv> `](/apis/Classes/HH/ImmVector/) - An [` ImmVector `](/apis/Classes/HH/ImmVector/) that is a subset of the current [` ImmVector `](/apis/Classes/HH/ImmVector/)
  starting with the value for which the callback first returns
  `` false ``.




## Examples




See [` Vector::skipWhile `](/apis/Classes/HH/Vector/skipWhile/#examples) for usage examples.
<!-- HHAPIDOC -->
