<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function nullthrows<T>(?T $x): T {
  return $x as nonnull;
}

abstract class PC<+T> {
}
interface I { }
abstract class PCF {

  final public async function foo(
    I $ent,
  ): Awaitable<PC<I>> {
    $c = await $this->bar($ent);
    return nullthrows($c);
  }

  final public async function bar(
    I $ent,
  ): Awaitable<?PC<I>> {
      return null;
  }
}
