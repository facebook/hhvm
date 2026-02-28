<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

interface IFunc {}
abstract class Func<Ta, Tr> implements IFunc {
  public abstract function apply(Ta $_): Tr;
}

class Even extends Func<(int), bool> {
  public function apply((int) $x): bool {
    return $x[0] % 2 === 0;
  }
}

class Length extends Func<(vec<int>), int> {
  public function apply((vec<int>) $x): int {
    return 0;
  }
}

class Mul extends Func<(int, int), int> {
  public function apply((int, int) $p): int {
    return $p[0] * $p[1];
  }
}

enum class Funcs: IFunc {
  Func<(int), bool> Even = new Even();
  Func<(vec<int>), int> Length = new Length();
  Func<(int, int), int> Mul = new Mul();
}

class CL {
  public function app<Ta, Tr>(
    \HH\EnumClass\Label<Funcs, Func<Ta, Tr>> $l,
    Ta $arg,
  ): Tr {
    // Do some generic stuff too
    return Funcs::valueOf($l)->apply($arg);
  }
}

<<__EntryPoint>>
function testcalling(): void {
  $lab = #Mul;
  $c = new CL();
  $r = $c->app($lab, tuple(2, 4));
  //       ^ hover-at-caret
}
