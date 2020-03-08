//<?hh // strict

namespace NS_scope_resolution;

class M {
// The following members are overridden in class P

  public static function psf(): string { return "red"; }
  public static int $psProp = -333;
  const float MYPI = 3.14;
  const int CON1 = -222;

// method b1 demostrates self vs. static

  public function b1(): void {
    echo "  self::\$psf returns " . self::psf() . "\n";
    echo "static::\$psf returns " . static::psf() . "\n";
    echo "  self::\$psProp is   " . self::$psProp . "\n";
    echo "static::\$psProp is   " . static::$psProp . "\n";
    echo "  self::\$MYPI is     " . self::MYPI . "\n";
    echo "static::\$MYPI is     " . static::MYPI . "\n";

    echo "  self::\$b2 --- "; self::b2();
    echo "static::\$b2 --- "; static::b2();
  }

  public function b2(): void {
    echo "Inside " . __METHOD__ . "\n";
  }

  public function __construct() { }
}

class N extends M {
  public function b2(): void	// overrides base::b2() {
    echo "Inside " . __METHOD__ . "\n";
  }
}

class P extends N {
  private int $prop;
  const int CON1 = 543;
  private int $prop2 = self::CON1;
  private int $prop3 = parent::CON1;
//  private $prop4 = static::CON1; // "static::" is not allowed in compile-time constants

  public function __construct(int $p1) {
    $this->prop = $p1;

    echo "Inside instance " . __METHOD__ . "\n";
    parent::__construct();
    M::__construct();		// allowed
    N::__construct();		// allowed
    $clName = "M";
    $clName::__construct();	// allowed
//    "M"::__construct();		// not allowed

// can call instance and static methods using both -> and ::

    $this->gi();	// $this explicitly used (and passed)
    P::gi();		// $this implicitly used (and passed)
    self::gi();		// $this implicitly used (and passed)

    $this->gs();	// call to static method, so no $this passed
    P::gs();		// call to static method, so no $this passed
    self::gs();		// call to static method, so no $this passed
  }

  public function gi(): void {
    echo "Inside instance " . __METHOD__ . "\n";
    \var_dump($this);
  }

  public static function gs(): void {
    echo "Inside static " . __METHOD__ . "\n";
  }

// method f1 demostrates self and parent

  public static function f1(): void {
    echo "Accessing self:\n";
    echo "psf returns " . self::psf() . "\n";
    echo "psProp = " . self::$psProp . "\n";
    echo "MYPI   = " . self::MYPI . "\n";

    echo "Accessing parent(s)\n";
    echo "psf returns " . parent::psf() . "\n";
    echo "psProp = " . parent::$psProp . "\n";
    echo "MYPI   = " . parent::MYPI . "\n";
  }

  public function b2(): void	// overrides base::b2() {
    echo "Inside " . __METHOD__ . "\n";
  }

// The following 3 members override those in class M

  public static function psf(): int { return 123; }
  public static int $psProp = 999;
  const MYPI = 3.14159;
}

// simplified version of static

class Base {
  public function b(): void {
//    echo "Inside " . __METHOD__ . "\n";
    static::f();
    echo "The static context here is " . static::class . "\n";
  }

  public function f(): void {
    echo "Inside " . __METHOD__ . "\n";
  }
}

class Derived extends Base {
  public function f(): void {
    echo "Inside " . __METHOD__ . "\n";
  }
}

interface I1 {
  const int CON1 = 123;
  const int CON2 = I1::CON1;
  const int CON3 = self::CON1;
  public function f(): void;
}

interface I2 extends I1 {
  const int CON4 = parent::CON1;	// Cannot access parent:: when current class scope has no parent
}

class X implements I2 { public function f(): void {} }

class W1 {}

class W2 extends W1 {
  private int $prop1 = 123;

  public function M(): void {
    echo "inside " . __METHOD__ . "\n";
    \var_dump($this);

    \var_dump(self::class);
    \var_dump(parent::class);
    \var_dump(static::class);
  }
}

function main(): void {
  \var_dump(M::psf());		// okay to access static method via class
  $memName = 'psf';
  \var_dump(M::$memName());// string form allowed
//  var_dump(M::'psf'());	// but not string literal

  \var_dump(M::$psProp);	// okay to access static property via class, but leading $ needed!!
  $memName = '$psProp';
//  var_dump(M::$memName);// Access to undeclared static property: M::$memName
//  var_dump(M::memName);// Undefined class constant 'memName'

  \var_dump(M::MYPI);		// okay to access const via class
  $memName = 'MYPI';
//  var_dump(M::$memName);	// Access to undeclared static property: M::$memName

  $clName = "M";
  \var_dump($clName::MYPI);	// okay to use a string containing class name
//  var_dump("M"::MYPI);		// but not the literal form

  echo "-----------------------------\n";

  \var_dump(P::psf());		// okay to access static method via class
  \var_dump(P::$psProp);	// okay to access static property via class, but leading $ needed!!
  \var_dump(P::MYPI);		// okay to access const via class

  $clName = "P";
  \var_dump($clName::MYPI);	// okay to use a string containing class name

  echo "-----------------------------\n";

  P::f1();

  echo "-----------------------------\n";

  $m = new M();
  $n = new N();
  $p = new P(1000);

  echo "-----------------------------\n";

  $m->b1();

  echo "-----------------------------\n";

  $n->b1();

  echo "-----------------------------\n";

  $p->b1();

  echo "-----------------------------\n";

  $b1 = new Base();
  $b1->b();
  $d1 = new Derived();
  $d1->b();

  echo "-----------------------------\n";

// see about :: in interfaces

  \var_dump(I1::CON1);
  $intName = 'I1';
  \var_dump($intName::CON1);
  \var_dump(I1::CON2);
  \var_dump(I1::CON3);

//  var_dump(I2::CON4);

//  $x = new X();

  echo "-----------------------------\n";

// see about ...::class

  $w2 = new W2();
  \var_dump($w2);

  $w2->M();

  \var_dump(W2::class);
}

/* HH_FIXME[1002] call to main in strict*/
main();
