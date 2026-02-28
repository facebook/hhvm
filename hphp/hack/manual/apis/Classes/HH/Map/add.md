
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Add a key/value pair to the end of the current [` Map `](/apis/Classes/HH/Map/)




``` Hack
public function add(
  Pair<Tk, Tv> $val,
): Map<Tk, Tv>;
```




This method is equivalent to [` Map::set() `](/apis/Classes/HH/Map/set/). If the key in the  [` Pair `](/apis/Classes/HH/Pair/)
exists in the [` Map `](/apis/Classes/HH/Map/),  the value associated with it is overwritten.




` $map->add($p) ` is equivalent to both `` $map[$k] = $v `` and
``` $map[] = Pair {$k, $v} ``` (except that [` add() `](/apis/Classes/HH/Map/add/) returns the [` Map `](/apis/Classes/HH/Map/)).




Future changes made to the current [` Map `](/apis/Classes/HH/Map/) ARE reflected in the returned
[` Map `](/apis/Classes/HH/Map/), and vice-versa.




## Parameters




+ [` Pair<Tk, `](/apis/Classes/HH/Pair/)`` Tv> $val ``




## Returns




* [` Map<Tk, `](/apis/Classes/HH/Map/)`` Tv> `` - Returns itself.




## Examples




The following example adds a single key-value pair to the [` Map `](/apis/Classes/HH/Map/) `` $m `` and also adds multiple key-value pairs to ``` $m ``` through chaining. Since [` Map::add() `](/apis/Classes/HH/Map/add/) returns a [shallow copy](<https://en.wikipedia.org/wiki/Object_copying#Shallow_copy>) of ` $m ` itself, you can chain a bunch of [` add() `](/apis/Classes/HH/Map/add/) calls together, and that will add all those values to `` $m ``.




``` basic-usage.hack
$m = Map {};

$m->add(Pair {'red', '#ff0000'});
\var_dump($m);

// Map::add returns the Map so it can be chained
$m->add(Pair {'green', '#00ff00'})
  ->add(Pair {'blue', '#0000ff'})
  ->add(Pair {'yellow', '#ffff00'});
\var_dump($m);
```
<!-- HHAPIDOC -->
