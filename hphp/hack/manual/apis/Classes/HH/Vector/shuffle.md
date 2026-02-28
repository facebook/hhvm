
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Shuffles the values of the current [` Vector `](/apis/Classes/HH/Vector/) randomly in place




``` Hack
public function shuffle(): void;
```




## Returns




+ ` void `




## Examples




~~~ basic-usage.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};

// Randomize the Vector elements in place
$v->shuffle();

\var_dump($v);
```.hhvm.expectf
object(HH\Vector) (4) {
  [0]=>
  string(%d) "%s"
  [1]=>
  string(%d) "%s"
  [2]=>
  string(%d) "%s"
  [3]=>
  string(%d) "%s"
}
```.example.hhvm.out
object(HH\Vector) (4) {
  [0]=>
  string(4) "blue"
  [1]=>
  string(5) "green"
  [2]=>
  string(6) "yellow"
  [3]=>
  string(3) "red"
}
~~~
<!-- HHAPIDOC -->
