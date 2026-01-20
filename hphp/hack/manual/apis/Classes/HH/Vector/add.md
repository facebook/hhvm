
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Appends a value to the end of the current [` Vector `](/apis/Classes/HH/Vector/), assigning it the next
available integer key




``` Hack
public function add(
  Tv $value,
): Vector<Tv>;
```




If you want to overwrite the value for an existing key, use [` set() `](/apis/Classes/HH/Vector/set/).




` $vec->add($v) ` is semantically equivalent to `` $vec[] = $v `` (except that
[` add() `](/apis/Classes/HH/Vector/add/) returns the current [` Vector `](/apis/Classes/HH/Vector/)).




Future changes made to the current [` Vector `](/apis/Classes/HH/Vector/) ARE reflected in the
returned [` Vector `](/apis/Classes/HH/Vector/), and vice-versa.




If ` $v ` is an object, future changes to the added element ARE reflected in
`` $v ``, and vice versa.




## Parameters




+ ` Tv $value `




## Returns




* [` Vector<Tv> `](/apis/Classes/HH/Vector/) - Returns itself.




## Examples




The following example adds a single value to the [` Vector `](/apis/Classes/HH/Vector/) `` $v `` and also adds multiple values to ``` $v ``` through chaining. Since [` Vector::add() `](/apis/Classes/HH/Vector/add/) returns a [shallow copy](<https://en.wikipedia.org/wiki/Object_copying#Shallow_copy>) of ` $v ` itself, you can chain a bunch of [` add() `](/apis/Classes/HH/Vector/add/) calls together, and that will add all those values to `` $v ``.




``` basic-usage.hack
$v = Vector {};

$v->add('red');
\var_dump($v);

// Vector::add returns the Vector so it can be chained
$v->add('green')
  ->add('blue')
  ->add('yellow');
\var_dump($v);
```
<!-- HHAPIDOC -->
