
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Stores a value into the current [` Vector `](/apis/Classes/HH/Vector/) with the specified key,
overwriting the previous value associated with the key




``` Hack
public function set(
  int $key,
  Tv $value,
): Vector<Tv>;
```




If the key is not present, an exception is thrown. If you want to add
a value even if the key is not present, use [` add() `](/apis/Classes/HH/Vector/add/).




` $vec->set($k,$v) ` is semantically equivalent to `` $vec[$k] = $v `` (except
that [` set() `](/apis/Classes/HH/Vector/set/) returns the current [` Vector `](/apis/Classes/HH/Vector/)).




Future changes made to the current [` Vector `](/apis/Classes/HH/Vector/) ARE reflected in the
returned [` Vector `](/apis/Classes/HH/Vector/), and vice-versa.




## Parameters




+ ` int $key `
+ ` Tv $value `




## Returns




* [` Vector<Tv> `](/apis/Classes/HH/Vector/) - Returns itself.




## Examples




Since [` Vector::set() `](/apis/Classes/HH/Vector/set/) returns a [shallow copy](<https://en.wikipedia.org/wiki/Object_copying#Shallow_copy>) of ` $v ` itself, you can chain a bunch of [` set() `](/apis/Classes/HH/Vector/set/) calls together.




``` basic-usage.hack
$v = Vector {'red', 'green', 'blue', 'yellow'};

// Set the first element to 'RED'
$v->set(0, 'RED');

\var_dump($v);

// Set the second and third elements using chaining
$v->set(1, 'GREEN')
  ->set(2, 'BLUE');

\var_dump($v);
```
<!-- HHAPIDOC -->
