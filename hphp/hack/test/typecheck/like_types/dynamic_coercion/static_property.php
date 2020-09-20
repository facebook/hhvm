<?hh

class C {
  const dynamic D = 4;
  public static int $sprop = self::D;
}

function f(): void {
  C::$sprop = C::D;
}
