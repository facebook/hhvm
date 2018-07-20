<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class A {

  abstract const type TField;
  abstract const type TActsOnEntType;

  public static function getFieldComment(): ?string {
    $ts = type_structure(static::class, 'TActsOnEntType');
    $s = $ts['classname'];
    return $s;
  }
}
