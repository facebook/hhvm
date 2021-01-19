<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

abstract class A<-TContext> {
  final public function gen(
  ): this::TFeatureValue {
    throw new Exception();
  }
  public function expect(?this::TFeatureValue $_):void { }
  abstract public static function set(?this::TFeatureValue $value): void;
  abstract const type TFeatureValue;
}


final class FC<T> {
  public function __construct(
    public dict<string, A<T>> $fld,
  ) {
  }
  private function test1(
  ): void {
    $impl = idx($this->fld, 'a');
    $value = $impl?->gen();
    if ($impl !== null) {
      $impl::set($value);
    }
  }
  private function test2():void {
    $impl = idx($this->fld, 'a');
    if ($impl !== null) {
      $value = $impl->gen();
      $impl::set($value);
    }
  }
}
