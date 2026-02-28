
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

:::warning
**Deprecated:** Use `new Map($arr)` instead.
:::




Returns a [` Map `](/apis/Classes/HH/Map/) containing the key/value pairs from the specified `` array ``




``` Hack
public static function fromArray(
  darray<Tk, Tv> $arr,
): Map<Tk, Tv>;
```




This function is deprecated. Use ` new `Map`` ($arr) `` instead.




## Parameters




+ ` darray<Tk, Tv> $arr ` - The `` array `` to convert to a [` Map `](/apis/Classes/HH/Map/).




## Returns




* [` Map<Tk, `](/apis/Classes/HH/Map/)`` Tv> `` - A [` Map `](/apis/Classes/HH/Map/) with the key/value pairs from the provided `` array ``.
<!-- HHAPIDOC -->
