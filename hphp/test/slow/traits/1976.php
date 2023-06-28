<?hh

trait Counter {

  private static $incC = 0;
  public function inc() :mixed{
    self::$incC = self::$incC + 1;
    $c = self::$incC;
    echo "$c\n";
  }
}
class C1 {
  use Counter;
}
class C2 {
  use Counter;
}
<<__EntryPoint>> function main(): void {
$o = new C1();
$o->inc();
$p = new C2();
$p->inc();
$o->inc();
$p->inc();
}
