<?hh // strict

namespace NS_void;

function v(): void {}

class C {
//  const void XX = true;

// interesting behavior below
/*

  private void $prop;	// surprise!

  public function __construct() {
    $this->prop = v();	// the default (null) value "returned" from a void function is used
  }
*/
//  public function setProp(void $val): void {
//    $this->prop = $val;
//  }

  public function getProp(): void {
  }
}

function main(): void {
  $c = new C();
  var_dump($c);
}

/* HH_FIXME[1002] call to main in strict*/
main();
