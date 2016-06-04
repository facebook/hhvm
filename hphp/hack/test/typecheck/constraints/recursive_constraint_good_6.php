<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface IEB {}
interface IVCB {}

interface IESEM<Tobj, Tvc as IVCB> extends IESM<Tobj, Tvc> {}

interface IESM<Tobj, Tvc as IVCB> extends ISM {}

interface ISM {}

final class C {

  public function Foo<Tent as IEB, Tvc as IVCB>(
    IESEM<Tent, Tvc> $x,
    Tent $ent,
  ): void {
    self::Bar($x, $ent);
  }

  public static function Bar<Tent as IEB, Tvc as IVCB>(
    IESEM<Tent, Tvc> $x,
    Tent $ent,
  ): void {}
}
