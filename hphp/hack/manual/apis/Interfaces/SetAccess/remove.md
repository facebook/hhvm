
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Removes the provided value from the current [` Set `](/apis/Classes/HH/Set/)




``` Hack
public function remove(
  Tm $m,
): this;
```




If the value is not in the current [` Set `](/apis/Classes/HH/Set/), the [` Set `](/apis/Classes/HH/Set/) is unchanged.




It the current [` Set `](/apis/Classes/HH/Set/), meaning changes  made to the current [` Set `](/apis/Classes/HH/Set/) will be
reflected in the returned [` Set `](/apis/Classes/HH/Set/).




## Parameters




+ ` Tm $m ` - The value to remove.




## Returns




* ` this ` - Returns itself.
<!-- HHAPIDOC -->
