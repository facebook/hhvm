
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Add the value to the current [` Set `](/apis/Classes/HH/Set/)




``` Hack
public function add(
  Tv $val,
): Set<Tv>;
```




` $set->add($v) ` is semantically equivalent to `` $set[] = $v `` (except that
[` add() `](/apis/Classes/HH/Set/add/) returns the [` Set `](/apis/Classes/HH/Set/)).




Future changes made to the current [` Set `](/apis/Classes/HH/Set/) ARE reflected in the returned
[` Set `](/apis/Classes/HH/Set/), and vice-versa.




## Parameters




+ ` Tv $val `




## Returns




* [` Set<Tv> `](/apis/Classes/HH/Set/) - Returns itself.




## Examples




The following example adds a single value to the [` Set `](/apis/Classes/HH/Set/) `` $s `` and also adds multiple values to ``` $s ``` through chaining. Since [` Set::add() `](/apis/Classes/HH/Set/add/) returns a [shallow copy](<https://en.wikipedia.org/wiki/Object_copying#Shallow_copy>) of ` $s ` itself, you can chain a bunch of [` add() `](/apis/Classes/HH/Set/add/) calls together, and that will add all those values to `` $s ``. Notice that adding a value that already exists in the [` Set `](/apis/Classes/HH/Set/) has no effect.




``` basic-usage.hack
$s = Set {};

$s->add('red');
\var_dump($s);

// Set::add returns the Set so it can be chained
$s->add('green')
  ->add('blue')
  ->add('yellow');
\var_dump($s);

// Adding an element that already exists in the Set has no effect
$s->add('green')
  ->add('blue')
  ->add('yellow');
\var_dump($s);
```
<!-- HHAPIDOC -->
