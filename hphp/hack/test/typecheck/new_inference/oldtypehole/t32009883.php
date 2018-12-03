<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I {}

abstract class C {

  abstract const type Tin as I;

  abstract public static function getClassname(
  ): classname<this::Tin>;

  final public static function coerce<Tout>(
    I $x,
  ): ?Tout {
    $c = static::getClassname();
    return $x instanceof $c ? $x : null;
  }
}
