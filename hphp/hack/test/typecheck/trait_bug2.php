<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface IEB { }

interface IEU {
  abstract const type TE as IEB;
}

interface IECWSS<TGE as IEB> extends GECI<TGE> {
}

interface GECI<+TGE as IEB> {
  abstract const type TID;
}

abstract final class SMCC {
  final public static function foo<TGE as IEB>(
    classname<IECWSS<TGE>> $e,
  ): void {
  }
}

abstract class ECI implements GECI<this::TGE> {
  abstract const type TU as IEU;
  abstract const type TGE as this::TU::TE;
}

trait TR {
  require implements IECWSS<this::TGE>;
  require extends ECI;

  protected static function bar(
  ): void {
    SMCC::foo(static::class);
  }
}
