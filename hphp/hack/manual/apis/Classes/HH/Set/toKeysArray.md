
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an ` array ` containing the values from the current [` Set `](/apis/Classes/HH/Set/)




``` Hack
public function toKeysArray(): varray<Tv>;
```




[` Set `](/apis/Classes/HH/Set/)s don't have keys. So this method just returns the values.




This method is interchangeable with [` toValuesArray() `](/apis/Classes/HH/Set/toValuesArray/).




## Returns




+ ` varray<Tv> ` - an integer-indexed `` array `` containing the values from the
  current [` Set `](/apis/Classes/HH/Set/).




## Examples




This example shows that ` toKeysArray ` is the same as [` toValuesArray `](/apis/Classes/HH/Set/toValuesArray/) because [` Set `](/apis/Classes/HH/Set/)s don't have keys:




``` basic-usage.hack
$s = Set {'red', 'green', 'blue', 'yellow'};

$keys_array = $s->toKeysArray();

\var_dump($keys_array === $s->toValuesArray());
\var_dump($keys_array);
```
<!-- HHAPIDOC -->
