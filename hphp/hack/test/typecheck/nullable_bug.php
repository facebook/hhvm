<?hh //strict
// Copyright 2004-present Facebook. All Rights Reserved.

/* HH_FIXME[4336] */
function nullthrows<T>(?T $x): T {
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
