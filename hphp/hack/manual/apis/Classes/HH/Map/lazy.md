
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a lazy, access elements only when needed view of the current
[` Map `](/apis/Classes/HH/Map/)




``` Hack
public function lazy(): KeyedIterable<Tk, Tv>;
```




Normally, memory is allocated for all of the elements of the [` Map `](/apis/Classes/HH/Map/). With
a lazy view, memory is allocated for an element only when needed or used
in a calculation like in [` map() `](/apis/Classes/HH/Map/map/) or [` filter() `](/apis/Classes/HH/Map/filter/).




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Returns




* [` KeyedIterable<Tk, `](/apis/Interfaces/HH/KeyedIterable/)`` Tv> `` - a [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) representing the lazy view into the current
  [` Map `](/apis/Classes/HH/Map/).




## Examples




This example shows you how to use [` lazy() `](/apis/Classes/HH/Map/lazy/) on a rather large [` Map `](/apis/Classes/HH/Map/) and the time for both a *strict* and *non-strict* version. Since we only need 5 of the elements in the end, the lazy view actually allows us to stop after we meet our required 5 without having to actually filter and allocate all 1000000 elements up front.




~~~ basic-usage.hack
$map = new Map(\range(0, 1000000));

$s = \microtime(true);
$non_lazy = $map->filter($x ==> $x % 2 === 0)->take(5);
$e = \microtime(true);

\var_dump($non_lazy);
echo "Time non-lazy: ".\strval($e - $s).\PHP_EOL;

// Using a lazy view of the Map can save us a bunch of time, possibly even
// cutting this call time by 90%.
$s = \microtime(true);
$lazy = $map->lazy()->filter($x ==> $x % 2 === 0)->take(5);
$e = \microtime(true);

\var_dump(new Map($lazy));
echo "Time lazy: ".\strval($e - $s).\PHP_EOL;
```.hhvm.expectf
object(HH\Map) (5) {
  [0]=>
  int(0)
  [2]=>
  int(2)
  [4]=>
  int(4)
  [6]=>
  int(6)
  [8]=>
  int(8)
}
Time non-lazy: %f
object(HH\Map) (5) {
  [0]=>
  int(0)
  [2]=>
  int(2)
  [4]=>
  int(4)
  [6]=>
  int(6)
  [8]=>
  int(8)
}
Time lazy: %f
```.example.hhvm.out
object(HH\Map) (5) {
  [0]=>
  int(0)
  [2]=>
  int(2)
  [4]=>
  int(4)
  [6]=>
  int(6)
  [8]=>
  int(8)
}
Time non-lazy: 0.11302947998047
object(HH\Map) (5) {
  [0]=>
  int(0)
  [2]=>
  int(2)
  [4]=>
  int(4)
  [6]=>
  int(6)
  [8]=>
  int(8)
}
Time lazy: 0.0068733692169189
~~~
<!-- HHAPIDOC -->
