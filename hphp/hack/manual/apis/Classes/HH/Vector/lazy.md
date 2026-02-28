
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a lazy, access-elements-only-when-needed view of the current
[` Vector `](/apis/Classes/HH/Vector/)




``` Hack
public function lazy(): KeyedIterable<int, Tv>;
```




Normally, memory is allocated for all of the elements of the [` Vector `](/apis/Classes/HH/Vector/).
With a lazy view, memory is allocated for an element only when needed or
used in a calculation like in [` map() `](/apis/Classes/HH/Vector/map/) or [` filter() `](/apis/Classes/HH/Vector/filter/).




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Returns




* [` KeyedIterable<int, `](/apis/Interfaces/HH/KeyedIterable/)`` Tv> `` - An integer-keyed [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) representing the lazy view into
  the current [` Vector `](/apis/Classes/HH/Vector/).




## Examples




This example shows you how to use [` lazy() `](/apis/Classes/HH/Vector/lazy/) on a rather large [` Vector `](/apis/Classes/HH/Vector/) and the time for both a *strict* and *non-strict* version. Since we only need 5 of the elements in the end, the lazy view actually allows us to stop after we meet our required 5 without having to actually filter and allocate all 1000000 elements up front.




~~~ basic-usage.hack
$vector = new Vector(\range(0, 1000000));

$s = \microtime(true);
$non_lazy = $vector->filter($x ==> $x % 2 === 0)->take(5);
$e = \microtime(true);

\var_dump($non_lazy);
echo "Time non-lazy: ".\strval($e - $s).\PHP_EOL;

// Using a lazy view of the vector can save us a bunch of time, possibly even
// cutting this call time by 90%.
$s = \microtime(true);
$lazy = $vector->lazy()->filter($x ==> $x % 2 === 0)->take(5);
$e = \microtime(true);

\var_dump(new Vector($lazy));
echo "Time lazy: ".\strval($e - $s).\PHP_EOL;
```.hhvm.expectf
object(HH\Vector) (5) {
  [0]=>
  int(0)
  [1]=>
  int(2)
  [2]=>
  int(4)
  [3]=>
  int(6)
  [4]=>
  int(8)
}
Time non-lazy: %f
object(HH\Vector) (5) {
  [0]=>
  int(0)
  [1]=>
  int(2)
  [2]=>
  int(4)
  [3]=>
  int(6)
  [4]=>
  int(8)
}
Time lazy: %f
```.example.hhvm.out
object(HH\Vector) (5) {
  [0]=>
  int(0)
  [1]=>
  int(2)
  [2]=>
  int(4)
  [3]=>
  int(6)
  [4]=>
  int(8)
}
Time non-lazy: 0.053816556930542
object(HH\Vector) (5) {
  [0]=>
  int(0)
  [1]=>
  int(2)
  [2]=>
  int(4)
  [3]=>
  int(6)
  [4]=>
  int(8)
}
Time lazy: 0.0069270133972168
~~~
<!-- HHAPIDOC -->
