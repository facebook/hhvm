
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values of this [` ImmSet `](/apis/Classes/HH/ImmSet/)




``` Hack
public function keys(): ImmVector<arraykey>;
```




[` ImmSet `](/apis/Classes/HH/ImmSet/)s don't have keys, so this will return the values.




This method is interchangeable with [` toImmVector() `](/apis/Classes/HH/ImmSet/toImmVector/) and [` values() `](/apis/Classes/HH/ImmSet/values/).




## Returns




+ [` ImmVector<arraykey> `](/apis/Classes/HH/ImmVector/) - an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values of the current [` ImmSet `](/apis/Classes/HH/ImmSet/).




## Examples




See [` Set::keys `](/apis/Classes/HH/Set/keys/#examples) for usage examples.
<!-- HHAPIDOC -->
