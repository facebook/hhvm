<?hh

trait T1 {} // allowed to be empty

class C1 {
  use T1;
}

trait T2a {
  public function f(): void {
    echo "Inside ".__TRAIT__."\n";
    echo "Inside ".__CLASS__."\n";
    echo "Inside ".__METHOD__."\n";
  }
}

trait T2b {
  //  function f($p1, $p2) // signatures not factored in when looking for name clashes
  public function f(): void {
    echo "Inside ".__TRAIT__."\n";
    echo "Inside ".__CLASS__."\n";
    echo "Inside ".__METHOD__."\n";
  }
}

class C2Base {
  public function f(): void {
    echo "Inside ".__METHOD__."\n";
  }
}

trait T3 {
  public function m1(): void {
    echo "Inside ".__METHOD__."\n";
  }
  protected function m2(): void {
    echo "Inside ".__METHOD__."\n";
  }
  private function m3(): void {
    echo "Inside ".__METHOD__."\n";
  }

  public function m4(): void {
    echo "Inside ".__METHOD__."\n";
  }
}

class C3 {
  use T3;
}


trait Tx1 {
  public function k(): void {
    echo "Inside ".__TRAIT__."\n";
    echo "Inside ".__CLASS__."\n";
    echo "Inside ".__METHOD__."\n";
  }
}

trait Tx2 {
  public function m(): void {
    echo "Inside ".__TRAIT__."\n";
    echo "Inside ".__CLASS__."\n";
    echo "Inside ".__METHOD__."\n";
  }
}

trait T5 {
  public static mixed $prop;
}

class C5a {
  use T5;
}

class C5b {
  use T5;
}

trait T6 {

  private static int $fV = 0;
  public function f(): void {
    echo "Inside ".__METHOD__."\n";
    echo "\$v = ".self::$fV."\n";
    self::$fV += 1;
  }
}

class C6a {
  use T6;
}

class C6b {
  use T6;
}

trait T7 {
  public static mixed $pubs = 123;

  public function f(): void {
    echo "Inside ".__TRAIT__."\n";
    echo "Inside ".__CLASS__."\n";
    echo "Inside ".__METHOD__."\n";
    var_dump($this);
  }

  public static function g(): void {
    echo "Inside ".__TRAIT__."\n";
    echo "Inside ".__CLASS__."\n";
    echo "Inside ".__METHOD__."\n";
  }
}

trait T9a {
  public function compute(/* ... */): void { /* ... */ }
}

trait T9b {
  public function compute(/* ... */): void { /* ... */ }
}

trait T9c {
  public function sort(/* ... */): void { /* ... */ }
}

trait T10 {
  private mixed $prop1 = 1000;
  protected static mixed $prop2;
  public mixed $prop3;
  public function compute(): void {}
  public static function getData(): void {}
}
<<__EntryPoint>>
function entrypoint_traits(): void {

  /*
     +-------------------------------------------------------------+
     | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
     +-------------------------------------------------------------+
  */

  error_reporting(-1);

  echo "Inside >".__TRAIT__."<\n";
  echo "Inside >".__CLASS__."<\n";
  echo "Inside >".__METHOD__."<\n";
  echo "Inside >".__FUNCTION__."<\n";

  $c3 = new C3();
  //$c3->m1();        // accessible, by default, but not once protected
  //$c3->m2();        // inaccessible, by default
  //$c3->m3(); // inaccessible, by default
  $c3->m4(); // accessible, by default

  echo "===================== static properties =========================\n";

  C5a::$prop = 123;
  C5b::$prop = "red";
  echo C5a::$prop."\n"; // ==> 123
  echo C5b::$prop."\n"; // ==> red

  echo "===================== function statics =========================\n";

  $v1 = new C6a();
  $v1->f(); // method run twice with same $v
  $v1->f();

  echo "-------\n";

  $v2 = new C6b();
  $v2->f(); // method run three times with a different $v
  $v2->f();
  $v2->f();

  echo
    "===================== Using a Trait without a Class =========================\n";


  T7::g();

  /*
  echo "-------\n";
  var_dump(T7::pubs); // doesn't work for static properties
  */

  echo "===================== examples for spec =========================\n";
}
