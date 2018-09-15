<?hh

class Foo {
  function getPipedThing($thing): int {
    return $thing
      |> self::doSomething($$)
      |> BarManager::manageBarsAndBarlikes($$, $bar_comparator->getBarAttributes())
      |> self::finalizeBars($$);
  }
}
