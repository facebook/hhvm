
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Vector `](/docs/apis/Classes/HH/Vector/) containing the values of the current [` Set `](/docs/apis/Classes/HH/Set/)




``` Hack
public function keys(): Vector<arraykey>;
```




[` Set `](/docs/apis/Classes/HH/Set/)s don't have keys, so this will return the values.




This method is interchangeable with [` toVector() `](/docs/apis/Classes/HH/Set/toVector/) and [` values() `](/docs/apis/Classes/HH/Set/values/).




## Returns




+ [` Vector<arraykey> `](/docs/apis/Classes/HH/Vector/) - a [` Vector `](/docs/apis/Classes/HH/Vector/) (integer-indexed) containing the values of the
  current [` Set `](/docs/apis/Classes/HH/Set/).




## Examples




This example shows that [` keys() `](/docs/apis/Classes/HH/Set/keys/) returns a [` Vector `](/docs/apis/Classes/HH/Vector/) of the [` Set `](/docs/apis/Classes/HH/Set/)'s values because [` Set `](/docs/apis/Classes/HH/Set/)s don't have keys:




``` basic-usage.hack
$s = Set {'red', 'green', 'blue', 'yellow'};
\var_dump($s->keys());
```
<!-- HHAPIDOC -->
