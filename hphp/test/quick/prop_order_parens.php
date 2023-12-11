<?hh
class D {
public $prop = 300;
public function prop() :mixed{ return 350; }
public $yo = 500;
public function yo() :mixed{ return 550; }
public $blah = 600;
public function blah() :mixed{ return 650; }
}
class C {
public static $x = 100;
public static $y = vec[200];
public static $z = vec[];
public static function foo1() :mixed{ return 150; }
public static function foo2() :mixed{ return 250; }
public $bar1 = 400;
public function bar2() :mixed{ return 450; }
public $baz;
public $w = vec[];
public function __construct() {
$this->baz = new D();
$this->w[] = new D();
}
}
<<__EntryPoint>>
function entrypoint_prop_order_parens(): void {
  C::$z[] = new D();

  var_dump(C::$x);
  $x = 'foo1';
  var_dump(C::$x());

  var_dump((C::$y)[0]);
  $y = vec['foo2'];
  var_dump(C::$y[0]());

  var_dump(((C::$z)[0])->prop);
  var_dump(((C::$z)[0])->prop());

  $obj = new C;
  $x = vec['bar2'];
  var_dump($obj->$x[0]());

  $obj = new C;
  var_dump(($obj->w)[0]->yo);
  var_dump(($obj->w)[0]->yo());

  $obj = new C;
  $w = vec['baz'];
  var_dump($obj->$w[0]->blah());
}
