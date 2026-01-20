
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the results of applying an operation to
each key/value pair in the current [` ImmVector `](/apis/Classes/HH/ImmVector/)




``` Hack
public function mapWithKey<Tu>(
  (function(int, Tv): Tu) $callback,
): ImmVector<Tu>;
```




[` mapWithKey() `](/apis/Classes/HH/ImmVector/mapWithKey/)'s result contains a value for every key/value pair in the
current [` ImmVector `](/apis/Classes/HH/ImmVector/); unlike [` filterWithKey() `](/apis/Classes/HH/ImmVector/filterWithKey/), where only values whose
key/value pairs meet a certain criterion are included in the resulting
[` ImmVector `](/apis/Classes/HH/ImmVector/).




## Parameters




+ ` (function(int, Tv): Tu) $callback `




## Returns




* [` ImmVector<Tu> `](/apis/Classes/HH/ImmVector/) - An [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the results of applying a
  user-specified operation to each key/value pair of the current
  [` ImmVector `](/apis/Classes/HH/ImmVector/) in turn.




## Examples




See [` Vector::mapWithKey `](/apis/Classes/HH/Vector/mapWithKey/#examples) for usage examples.
<!-- HHAPIDOC -->
