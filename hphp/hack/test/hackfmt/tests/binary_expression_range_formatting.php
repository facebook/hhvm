<?hh

abstract final class FooClass {
  public static function getFirstPotentialBar() {
    $potentialBars = BarManager::getPotentialBarsForType($type) |>
      Vec\filter(
        $$,
        $index ==> self::isPotentialBar($index, $someOtherLongArgument)
      ) |>
      Vec\map(
        $$,
        (
          Baz::TSomeTypeWithALongIdentifier $index,
        ): ?SomePotentialBarType ==>
          self::getPotentialBar(
            $index,
            $argumentTwo,
            $argumentThree,
            $argumentFour,
          ),
      ) |>
      Vec\filter_nulls($$) |>
      Vec\sort(
        $$,
        (SomePotentialBarType $a, SomePotentialBarType $b): int ==>
          self::comparePotentialBars($a, $b),
      );

    return C\first($potentialBars);
  }
}
