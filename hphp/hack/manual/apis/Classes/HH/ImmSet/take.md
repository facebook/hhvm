
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmSet `](/apis/Classes/HH/ImmSet/) containing the first n values of the current [` ImmSet `](/apis/Classes/HH/ImmSet/)




``` Hack
public function take(
  int $n,
): ImmSet<Tv>;
```




The returned [` ImmSet `](/apis/Classes/HH/ImmSet/) will always be a proper subset of the current
[` ImmSet `](/apis/Classes/HH/ImmSet/).




` n ` is 1-based. So the first element is 1, the second 2, etc.




## Parameters




+ ` int $n ` - The last element that will be included in the returned
  [` ImmSet `](/apis/Classes/HH/ImmSet/).




## Returns




* [` ImmSet<Tv> `](/apis/Classes/HH/ImmSet/) - An [` ImmSet `](/apis/Classes/HH/ImmSet/) that is a proper subset of the current [` ImmSet `](/apis/Classes/HH/ImmSet/) up
  to `` n `` elements.




## Examples




See [` Set::take `](/apis/Classes/HH/Set/take/#examples) for usage examples.
<!-- HHAPIDOC -->
