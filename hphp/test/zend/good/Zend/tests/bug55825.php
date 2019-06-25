<?hh

trait T1 {
  private static $incX =1;
    public function inc() {
        echo self::$incX++ . "\n";
    }
}

class C { use T1; }

<<__EntryPoint>> function main(): void {
$c1 = new C;
$c1->inc();
$c1->inc();
}
