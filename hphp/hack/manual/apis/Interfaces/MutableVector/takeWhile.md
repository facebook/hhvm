
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableVector `](/docs/apis/Interfaces/MutableVector/) containing the values of the current
[` MutableVector `](/docs/apis/Interfaces/MutableVector/) up to but not including the first value that produces
`` false `` when passed to the specified callback




``` Hack
public function takeWhile(
  (function(Tv): bool) $fn,
): MutableVector<Tv>;
```




The returned [` MutableVector `](/docs/apis/Interfaces/MutableVector/) will always be a proper subset of the current
[` MutableVector `](/docs/apis/Interfaces/MutableVector/).




## Parameters




+ ` (function(Tv): bool) $fn ` - The callback that is used to determine the stopping
  condition.




## Returns




* [` MutableVector<Tv> `](/docs/apis/Interfaces/MutableVector/) - A [` MutableVector `](/docs/apis/Interfaces/MutableVector/) that is a proper subset of the current
  [` MutableVector `](/docs/apis/Interfaces/MutableVector/) up until the callback returns `` false ``.
<!-- HHAPIDOC -->
