
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values of the current [` ImmVector `](/apis/Classes/HH/ImmVector/) up
to but not including the first value that produces `` false `` when passed to
the specified callback




``` Hack
public function takeWhile(
  (function(Tv): bool) $callback,
): ImmVector<Tv>;
```




That is, takes the continuous prefix of values in
the current [` ImmVector `](/apis/Classes/HH/ImmVector/) for which the specified callback returns `` true ``.




The returned [` ImmVector `](/apis/Classes/HH/ImmVector/) will always be a subset (but not necessarily a
proper subset) of the current [` ImmVector `](/apis/Classes/HH/ImmVector/).




## Parameters




+ ` (function(Tv): bool) $callback `




## Returns




* [` ImmVector<Tv> `](/apis/Classes/HH/ImmVector/) - An [` ImmVector `](/apis/Classes/HH/ImmVector/) that is a subset of the current [` ImmVector `](/apis/Classes/HH/ImmVector/) up
  until when the callback returns `` false ``.




## Examples




See [` Vector::takeWhile `](/apis/Classes/HH/Vector/takeWhile/#examples) for usage examples.
<!-- HHAPIDOC -->
