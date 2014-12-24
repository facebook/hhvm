<?hh

class Thing {
  public static function filter<T>(
    $collection,
    $predicate,
  ): Traversable<T> {
    foreach ($collection as $v) {
      if ($predicate($v)) {
        yield $v;
      }
    }
  }
}

function funny($thing) {
  $blah = Thing::filter($thing, $x ==> true);

  foreach ($blah as $k) {
    if ($k !== null) echo $k;
  }
}

for ($i = 0; $i < 10; ++$i) {
  funny(Set { 'asd', 'bsd', 'foo', 'asd' });
}
