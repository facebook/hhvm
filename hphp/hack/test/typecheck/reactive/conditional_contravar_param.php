<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface IRxVarEvalBase {}
interface IRxVarEval extends IRxVarEvalBase {}

// subtype for testing variance rules
interface IRxVarEvalSub extends IRxVarEvalBase {}

abstract class RxVarCheckBase {
  private function __construct() {}

  <<__RxShallow, __AtMostRxAsArgs, __OnlyRxIfImpl(IRxVarCheck::class)>>
  protected static function contra(
    <<__OnlyRxIfImpl(IRxVarEval::class)>> IRxVarEvalSub $_
  ): void {}
}

abstract class RxVarCheckWithAttr extends RxVarCheckBase {
  <<
    __RxShallow,
    __OnlyRxIfImpl(IRxVarCheck::class),
    __AtMostRxAsArgs,
    __Override>>
  final protected static function contra(
    <<__OnlyRxIfImpl(IRxVarEval::class)>> // fix for E4341 at use site
    IRxVarEvalBase $_
  ): void {}
}

abstract class RxVarCheckWithoutAttr extends RxVarCheckBase {
  <<
    __RxShallow,
    __OnlyRxIfImpl(IRxVarCheck::class),
    __Override>>
  final protected static function contra(
    IRxVarEvalBase $_
  ): void {}
}

interface IRxVarCheck {
  require extends RxVarCheckBase;
}
