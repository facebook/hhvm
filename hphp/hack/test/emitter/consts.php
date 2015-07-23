<?hh // strict

class Lol {
  const int LOL = 5;

  public static function getLol(): int { return static::LOL; }
}

class Lol2 extends Lol {
  const int LOL = Lol::LOL;
  const int LOL2 = parent::LOL+1;
  const int LOL3 = self::LOL+2;

}

class Lol3 {
  const int LOL = Lol::LOL;
}

abstract class Abs {
  abstract const int X;
}
class AbsA extends Abs {
  const int X = 10;
}
class AbsB extends Abs {
  const int X = 100;
}
function getX(Abs $x): void {
  var_dump($x::X);
}

function test(): void {
  var_dump(Lol::LOL);
  var_dump(Lol2::LOL);
  var_dump(Lol2::LOL2);
  var_dump(Lol2::LOL3);
  var_dump(Lol3::LOL);

  var_dump(Lol::getLol());
  var_dump(Lol2::getLol());

  var_dump(AbsA::X);
  var_dump(AbsB::X);
  getX(new AbsA());
  getX(new AbsB());
}
