<?hh // strict

interface FunAsClass<Tin, Tout> {
  public function do(Tin $x): Tout;
}

class Good implements FunAsClass<dynamic, mixed> {
  public function do(mixed $x): mixed {
    return $x;
  }
}

class Bad implements FunAsClass<mixed, mixed> {
  public function do(dynamic $x): mixed {
    return $x;
  }
}
