
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an ` array ` whose values are the keys from the current [` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function toKeysArray(): varray<int>;
```




## Returns




+ ` varray<int> ` - An `` array `` with the integer keys from the current [` Vector `](/apis/Classes/HH/Vector/).




## Examples




``` basic-usage.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};

$keys_array = $v->toKeysArray();

\var_dump(\HH\is_any_array($keys_array));
\var_dump($keys_array);
```
<!-- HHAPIDOC -->
