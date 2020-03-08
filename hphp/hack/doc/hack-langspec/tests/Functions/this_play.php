<?hh // strict

namespace NS_this_play;

class Base {

// The type "this" can only be used as a return type, to instantiate a covariant type variable,
// or as a private non-static member variable

  private ?this $pr1;
/*
  public function f1(this): void {
  }
*/
  public function f2(): this {
    return $this;
  }

//  public function __construct() {
//    $this->pr1 = $this;
//  }

  private int $x = 0;

//  public function setX(int $new_x): Base {
  public function setX(int $new_x): this {
    $this->x = $new_x;
    // $this has type "this"
var_dump($this);
    return $this;
  }

/*
//  public static function newInstance(): Base {
  public static function newInstance(): this {
    // new static() has type "this"
    return new static();
  }
*/

/*
//  public function newCopy(): Base {
  public function newCopy(): this {
    // This would not type check with self::, but static:: is ok
    return static::newInstance();
  }
*/
/*
  // You can also say Awaitable<this>;
  public async function genThis(): \Awaitable<this> {
    return $this;
  }
*/
}

class Derived extends Base {}

final class Child {
  public function newChild(): this {
    // This is OK because Child is final.
    // However, if Grandchild extends Child, then this would be wrong, since
    // $grandchild->newChild() should returns a Child instead of a Grandchild
    return new Child();
  }
}

function main(): void {
  $b1 = new Base();
  $obj = $b1->setX(10);
  var_dump($obj);

  $d1 = new Derived();
  $obj = $d1->setX(20);
  var_dump($obj);
}

/* HH_FIXME[1002] call to main in strict*/
main();
