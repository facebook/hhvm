
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` Vector `](/apis/Classes/HH/Vector/) containing the results of applying an operation to each
key/value pair in the current [` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function mapWithKey<Tu>(
  (function(int, Tv): Tu) $callback,
): Vector<Tu>;
```




[` mapWithKey() `](/apis/Classes/HH/Vector/mapWithKey/)'s result contains a value for every key/value pair in the
current [` Vector `](/apis/Classes/HH/Vector/); unlike [` filterWithKey() `](/apis/Classes/HH/Vector/filterWithKey/), where only values whose
key/value pairs meet a certain criterion are included in the resulting
[` Vector `](/apis/Classes/HH/Vector/).




## Parameters




+ ` (function(int, Tv): Tu) $callback `




## Returns




* [` Vector<Tu> `](/apis/Classes/HH/Vector/) - A [` Vector `](/apis/Classes/HH/Vector/) containing the results of applying a user-specified
  operation to each key/value pair of the current [` Vector `](/apis/Classes/HH/Vector/) in turn.




## Examples




This example shows how ` mapWithKey ` can be used to create a new [` Vector `](/apis/Classes/HH/Vector/) based on `` $v ``'s keys and values:




``` basic-usage.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};

$sentences = $v->mapWithKey(($index, $color) ==> "Color at {$index}: {$color}");

echo \implode("\n", $sentences)."\n";
```
<!-- HHAPIDOC -->
