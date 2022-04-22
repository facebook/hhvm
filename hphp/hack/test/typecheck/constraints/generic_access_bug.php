<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface I {
  abstract const type TEnum as string;
  public static function get(): this;
  public function getEnumValue(): this::TEnum;
}

interface J { }
abstract class C<TU as I as J> {
  public function getClass():classname<TU> {
    throw new Exception("A");
  }
  final public function bar<T>(TU $u): T where T = TU::TEnum {
    $class = $this->getClass();
    return $class::get()->getEnumValue();
  }
}
