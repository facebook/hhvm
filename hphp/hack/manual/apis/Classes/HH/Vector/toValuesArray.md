
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an ` array ` containing the values from the current [` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function toValuesArray(): varray<Tv>;
```




This method is interchangeable with ` toArray() `.




## Returns




+ ` varray<Tv> ` - An `` array `` containing the values from the current [` Vector `](/apis/Classes/HH/Vector/).




## Examples




``` basic-usage.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};

$array = $v->toValuesArray();

\var_dump(\HH\is_any_array($array));
\var_dump($array);
```
<!-- HHAPIDOC -->
