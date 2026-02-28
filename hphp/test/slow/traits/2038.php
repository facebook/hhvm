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

<<__EntryPoint>>
function main_entry(): void {

  error_reporting(E_ALL);

  $o = new C1();
  $o->inc();
  $o->inc();

  $p = new C2();
  $p->inc();
  $p->inc();
}
