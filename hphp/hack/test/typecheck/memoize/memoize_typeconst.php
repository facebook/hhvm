<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

abstract class A implements IMemoizeParam { }

abstract class ATD {
  <<__Enforceable>>
  abstract const type TC2 as A;
}

abstract class ABase implements IAH {
  const type TC2 = this::TC::TC2;
}
interface IAH {
  abstract const type TC as ATD;
}

trait TR {
  require extends ABase;

  abstract const type TC;

  <<__Memoize>>
  public async function genThing(
    this::TC2 $asset_xid,
  ): Awaitable<void> { }
}
