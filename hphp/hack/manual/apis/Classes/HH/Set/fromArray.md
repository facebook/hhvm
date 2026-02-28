
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

:::warning
**Deprecated:** Use `new Set($arr)` instead.
:::




Returns a [` Set `](/apis/Classes/HH/Set/) containing the values from the specified `` array ``




``` Hack
public static function fromArray(
  darray<arraykey, Tv> $arr,
): Set<Tv>;
```




This function is deprecated. Use ` new Set ($arr) ` instead.




## Parameters




+ ` darray<arraykey, Tv> $arr ` - The `` array `` to convert to a [` Set `](/apis/Classes/HH/Set/).




## Returns




* [` Set<Tv> `](/apis/Classes/HH/Set/) - A [` Set `](/apis/Classes/HH/Set/) with the values from the provided `` array ``.
<!-- HHAPIDOC -->
