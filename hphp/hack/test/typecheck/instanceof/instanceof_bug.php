<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I<Tfb, Tsf, -T> {
}

abstract class S<Tfb, Tsf, T> implements I<Tfb, Tsf, T> {
}

abstract class SF<Tfb, Tsf, T> extends S<Tfb, Tsf, T> {
  final public function getBaseField(): S<Tfb, Tsf, T> {
    throw new Exception();
  }
}

abstract final class C {

  public static async function genEntityExample<T>(
    I<mixed, mixed, T> $f,
  ): Awaitable<mixed> {
    if ($f is SF<_, _, _>) {
      $x = $f->getBaseField();
      $y = await self::genEntityExample($x);
      return $y;
    }
    return null;
  }
}
