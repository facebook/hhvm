
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a lazy, access elements only when needed view of the current
[` Set `](/apis/Classes/HH/Set/)




``` Hack
public function lazy(): KeyedIterable<arraykey, Tv>;
```




Normally, memory is allocated for all of the elements of the [` Set `](/apis/Classes/HH/Set/). With
a lazy view, memory is allocated for an element only when needed or used
in a calculation like in [` map() `](/apis/Classes/HH/Set/map/) or [` filter() `](/apis/Classes/HH/Set/filter/).




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Returns




* [` KeyedIterable<arraykey, `](/apis/Interfaces/HH/KeyedIterable/)`` Tv> `` - an [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) representing the lazy view into the current
  [` Set `](/apis/Classes/HH/Set/), where the keys are the same as the values.




## Examples




This example shows you how to use [` lazy() `](/apis/Classes/HH/Set/lazy/) on a rather large [` Set `](/apis/Classes/HH/Set/) and the time for both a *strict* and *non-strict* version. Since we only need 5 of the elements in the end, the lazy view actually allows us to stop after we meet our required 5 without having to actually filter and allocate all 1000000 elements up front.




~~~ basic-usage.hack
$set = new Set(\range(0, 1000000));

$s = \microtime(true);
$non_lazy = $set->filter($x ==> $x % 2 === 0)->take(5);
$e = \microtime(true);

\var_dump($non_lazy);
echo "Time non-lazy: ".\strval($e - $s).\PHP_EOL;

// Using a lazy view of the Set can save us a bunch of time, possibly even
// cutting this call time by 90%.
$s = \microtime(true);
$lazy = $set->lazy()->filter($x ==> $x % 2 === 0)->take(5);
$e = \microtime(true);

\var_dump(new Set($lazy));
echo "Time lazy: ".\strval($e - $s).\PHP_EOL;
```.hhvm.expectf
object(HH\Set) (5) {
  int(0)
  int(2)
  int(4)
  int(6)
  int(8)
}
Time non-lazy: %f
object(HH\Set) (5) {
  int(0)
  int(2)
  int(4)
  int(6)
  int(8)
}
Time lazy: %f
```.example.hhvm.out
object(HH\Set) (5) {
  int(0)
  int(2)
  int(4)
  int(6)
  int(8)
}
Time non-lazy: 0.11553406715393
object(HH\Set) (5) {
  int(0)
  int(2)
  int(4)
  int(6)
  int(8)
}
Time lazy: 0.0063431262969971
~~~
<!-- HHAPIDOC -->
