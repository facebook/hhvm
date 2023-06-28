<?hh

trait T1 {

  <<__LSB>>
  private static $incX =0;
  public function inc($who) :mixed{
    static::$incX++;
    echo $who . " (" . __CLASS__ . "): " . static::$incX . "\n";
  }
}
class B {
 use T1;
 }
class C {
 use T1;
 }
class D extends C {
}

<<__EntryPoint>>
function main_2055() :mixed{
$c1 = new C;
$c2 = new C;
$d1 = new D;
$b1 = new B;
$c1->inc("c1");
$c2->inc("c2");
$d1->inc("d1");
$b1->inc("b1");
$b1->inc("b1");
$c2->inc("c2");
$d1->inc("d1");
$c1->inc("c1");
}
