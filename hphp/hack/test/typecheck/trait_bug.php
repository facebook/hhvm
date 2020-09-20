<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class QSIC extends QIC<this::TCoerced> {
  abstract const type TCoerced as nonnull;
}

<<__ConsistentConstruct>>
trait TQSIC {

  require extends QSIC;
  require extends QIC<this::TCoerced>;

  private function __construct() {}

  final public static function nonNullable(): this {
    return new static();
  }

  final public static function nullable(
  ): GNIC<this::TCoerced> {
    return new GNIC(static::nonNullable());
  }
}

final class GNIC<TInner as nonnull>
  extends QIC<?TInner> {

  public function __construct(
    private QIC<TInner> $innerCoercer,
  ) {}
}

abstract class QIC<+TCoerced> {}
