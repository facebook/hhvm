
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmSet `](/docs/apis/Classes/HH/ImmSet/) containing all the values from the specified
`` array ``(s)




``` Hack
public static function fromArrays(
  ...$argv,
): ImmSet<Tv>;
```




## Parameters




+ ` ...$argv ` - The `` array ``(s) to convert to an [` ImmSet `](/docs/apis/Classes/HH/ImmSet/).




## Returns




* [` ImmSet<Tv> `](/docs/apis/Classes/HH/ImmSet/) - An [` ImmSet `](/docs/apis/Classes/HH/ImmSet/) with the values from the passed `` array ``(s).




## Examples




See [` Set::fromArrays `](/docs/apis/Classes/HH/Set/fromArrays/#examples) for usage examples.
<!-- HHAPIDOC -->
