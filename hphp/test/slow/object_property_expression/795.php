<?hh
class C1 {
  private static $b = 10;
  public function __get( $what ) {

    return self::$b;
  }
}





<<__EntryPoint>>
function main_795() {

$c1 = new C1();
var_dump($c1->a);
$c1->a = 8;
var_dump($c1->a);
}
