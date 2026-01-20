
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmSet `](/apis/Classes/HH/ImmSet/) containing the values of the current [` ImmSet `](/apis/Classes/HH/ImmSet/) starting
after and including the first value that produces `` true `` when passed to
the specified callback




``` Hack
public function skipWhile(
  (function(Tv): bool) $fn,
): ImmSet<Tv>;
```




The returned [` ImmSet `](/apis/Classes/HH/ImmSet/) will always be a proper subset of the current
[` ImmSet `](/apis/Classes/HH/ImmSet/).




## Parameters




+ ` (function(Tv): bool) $fn ` - The callback used to determine the starting element for the
  [` ImmSet `](/apis/Classes/HH/ImmSet/).




## Returns




* [` ImmSet<Tv> `](/apis/Classes/HH/ImmSet/) - An [` ImmSet `](/apis/Classes/HH/ImmSet/) that is a proper subset of the current [` ImmSet `](/apis/Classes/HH/ImmSet/)
  starting after the callback returns `` true ``.




## Examples




See [` Set::skipWhile `](/apis/Classes/HH/Set/skipWhile/#examples) for usage examples.
<!-- HHAPIDOC -->
