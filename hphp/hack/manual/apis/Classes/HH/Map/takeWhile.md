
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Map `](/apis/Classes/HH/Map/) containing the keys and values of the current [` Map `](/apis/Classes/HH/Map/) up to
but not including the first value that produces `` false `` when passed to the
specified callback




``` Hack
public function takeWhile(
  (function(Tv): bool) $callback,
): Map<Tk, Tv>;
```




The returned [` Map `](/apis/Classes/HH/Map/) will always be a proper subset of the current [` Map `](/apis/Classes/HH/Map/).




## Parameters




+ ` (function(Tv): bool) $callback `




## Returns




* [` Map<Tk, `](/apis/Classes/HH/Map/)`` Tv> `` - A [` Map `](/apis/Classes/HH/Map/) that is a proper subset of the current [` Map `](/apis/Classes/HH/Map/) up until
  the callback returns `` false ``.
<!-- HHAPIDOC -->
