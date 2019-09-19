<?hh

enum Opaque: int {}

class C {
  const Opaque X = 3 as ~Opaque;
  public Opaque $y = 4 as ~Opaque;
  public static Opaque $z = 5 as ~Opaque;

  public function f(Opaque $w = 6 as ~Opaque): void {}
}

const Opaque D = 7 as ~Opaque;

function f(Opaque $i = 8 as ~Opaque): void {
  $j = 9 as ~Opaque;
}
