
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an ` array ` containing the values from the current [` ImmSet `](/apis/Classes/HH/ImmSet/)




``` Hack
public function toKeysArray(): varray<Tv>;
```




[` Set `](/apis/Classes/HH/Set/)s don't have keys. So this method just returns the values.




This method is interchangeable with [` toValuesArray() `](/apis/Classes/HH/ImmSet/toValuesArray/).




## Returns




+ ` varray<Tv> ` - an integer-indexed `` array `` containing the values from the
  current [` ImmSet `](/apis/Classes/HH/ImmSet/).




## Examples




See [` Set::toKeysArray `](/apis/Classes/HH/Set/toKeysArray/#examples) for usage examples.
<!-- HHAPIDOC -->
