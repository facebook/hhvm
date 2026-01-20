
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

:::warning
**Deprecated:** Use `new Vector($arr)` instead.
:::




Returns a [` Vector `](/apis/Classes/HH/Vector/) containing the values from the specified `` array ``




``` Hack
public static function fromArray(
  darray<arraykey, Tv> $arr,
): Vector<Tv>;
```




This function is deprecated. Use ` new Vector($arr) ` instead.




## Parameters




+ ` darray<arraykey, Tv> $arr ` - The `` array `` to convert to a [` Vector `](/apis/Classes/HH/Vector/).




## Returns




* [` Vector<Tv> `](/apis/Classes/HH/Vector/) - A [` Vector `](/apis/Classes/HH/Vector/) with the values from the provided `` array ``.
<!-- HHAPIDOC -->
