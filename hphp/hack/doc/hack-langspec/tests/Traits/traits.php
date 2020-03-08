<?hh // strict

namespace NS_traits;

///*
trait T1 {}		// allowed to be empty

class C1 { use T1; }

trait T2a {
  public function f(): void {
    echo "Inside " . __TRAIT__ . "\n";
    echo "Inside " . __CLASS__ . "\n";
    echo "Inside " . __METHOD__ . "\n";
  }
}
//*/

//*
trait T2b {
  public function f(): void {
    echo "Inside " . __TRAIT__ . "\n";
    echo "Inside " . __CLASS__ . "\n";
    echo "Inside " . __METHOD__ . "\n";
  }
}
//*/

///*
class C2Base {
  public function f(): void { echo "Inside " . __METHOD__ . "\n"; }
}
//*/
/*

class C2Derived extends C2Base {
//  use T2a, T2b	// Hack gags on  "{ ... }, so split into following 2 lines, but still gags
  use T2a;	// trait containing public function f(): void
  use T2b {		// trait containing public function f(): void
    T2a::f insteadof T2b;

    T2b::f as g;	// allow otherwise hidden T2B::f to be seen through alias g
    T2a::f as h;	// allow T2a::f to also be seen through alias h
			// don't need qualifier prefix if f is unambiguous
  }
}
*/

///*
trait T3 {
  public function m1(): void { echo "Inside " . __METHOD__ . "\n"; }
  protected function m2(): void { echo "Inside " . __METHOD__ . "\n"; }
  private function m3(): void { echo "Inside " . __METHOD__ . "\n"; }
  public function m4(): void { echo "Inside " . __METHOD__ . "\n"; }
}
//*/

/*
class C3 {
// Hack gags on "{ ... }

  use T3 {
    m1 as protected;		// reduce visibility to future, derived classes
    m2 as private;
    m3 as public;
    m3 as protected z3;
  }
}
*/

///*
trait Tx1 {
  public function k(): void {
    echo "Inside " . __TRAIT__ . "\n";
    echo "Inside " . __CLASS__ . "\n";
    echo "Inside " . __METHOD__ . "\n";
  }
}
//*/

///*
trait Tx2 {
  public function m(): void {
    echo "Inside " . __TRAIT__ . "\n";
    echo "Inside " . __CLASS__ . "\n";
    echo "Inside " . __METHOD__ . "\n";
  }
}
//*/

/*
trait T4 {
  use Tx1, Tx2;

// Hack gags on "{ ... }

  use T2a, T2b, T3 {
    Tx1::k as kk;
    T2a::f insteadof T2b;
  }
}
/*/

/*
class C4 {
  use T4;
}
*/

///*
trait T5 {
  public static num $prop = -99;
}
//*/

///*
class C5a {
  use T5;
}
//*/

///*
class C5b {
  use T5;
}
//*/

///*
trait T6 {
  public function f(): void {
    echo "Inside " . __METHOD__ . "\n";

    static $v = 0;			// static is class-specific
    echo "\$v = " . $v++ . "\n";
  }
}
//*/

///*
class C6a {
  use T6;
}
//*/

///*
class C6b {
  use T6;
}
//*/

///*
trait T7 {
  public static int $pubs = 123;

  public function f(): void {
    echo "Inside " . __TRAIT__ . "\n";
    echo "Inside " . __CLASS__ . "\n";
    echo "Inside " . __METHOD__ . "\n";
    var_dump($this);
  }

  public static function g(): void {
    echo "Inside " . __TRAIT__ . "\n";
    echo "Inside " . __CLASS__ . "\n";
    echo "Inside " . __METHOD__ . "\n";
  }
} 
//*/

///*
trait T9a {
  public function compute(): void { }
}
//*/

///*
trait T9b {
  public function compute(): void { }
}
//*/

///*
trait T9c {
  public function sort(): void { }
}
//*/

/*
trait T9d {
  use T9c;

// Hack gags on "{ ... }

  use T9a, T9b {
    T9a::compute insteadof T9b;
    T9c::sort as private sorter;
  }
}
*/

///*
trait T10 {
  private int $prop1 = 1000;
  protected static int $prop2 = -987;
//  var $prop3;
  public function compute(): void {}
  public static function getData(): void {}
}
//*/

function main(): void {
  echo "=== display some __***__ values ===\n\n";

//  echo "Inside >" . __TRAIT__ . "<\n";
//  echo "Inside >" . __CLASS__ . "<\n";
  echo "Inside >" . __METHOD__ . "<\n";
  echo "Inside >" . __FUNCTION__ . "<\n";
/*
  echo "\n=== Play with C2Derived ===\n\n";

  $c2 = new C2Derived();

  echo "-------\n";
  $c2->f();		// call T2a::f

  echo "-------\n";
  $c2->g();		// call T2b::f via its alias g

  echo "-------\n";
  $c2->h();		// call T2a::f via its alias h
*/
/*
  echo "\n=== Play with C3 ===\n\n";

// confirmed that lookup starts with current class, then trait(s), then base classes

  $c3 = new C3();
//  $c3->m1();		// accessible, by default, but not once protected
//  $c3->m2();		// inaccessible, by default
  $c3->m3();			// inaccessible, by default
  $c3->m4();			// accessible, by default
*/
/*
  echo "\n=== Play with C4 ===\n\n";

  $c4 = new C4();

  echo "-------\n";
  $c4->f();

  echo "-------\n";
  $c4->m1();

  echo "-------\n";
  $c4->k();

  echo "-------\n";
  $c4->m();
*/
///*
  echo "\n=== Play with C5a and C5b ===\n\n";

  C5a::$prop = 123;
  C5b::$prop = 43.56;
  echo C5a::$prop . "\n";	// ==> 123
  echo C5b::$prop . "\n";	// ==> 43.56
//*/
///*
  echo "\n=== Play with C6a ===\n\n";

  $v1 = new C6a();
  $v1->f();		// method run twice with same $v
  $v1->f();
//*/
///*
  echo "\n=== Play with C6b ===\n\n";

  $v2 = new C6b();
  $v2->f();		// method run three times with a different $v
  $v2->f();
  $v2->f();
//*/
///*
  echo "\n=== Play with T7 ===\n\n";

// Hack gags on this, but it's already "non-standard" in PHP
//  T7::f(); 	// calls f like a static function with class name being the trait name

  echo "-------\n";

  T7::g(); 
//*/
}

/* HH_FIXME[1002] call to main in strict*/
main();
