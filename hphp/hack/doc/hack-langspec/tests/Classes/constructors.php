<?hh // strict

namespace NS_constructors;

class D1 {
  public function __construct(int $p1) {
    echo "In D1 constructor, $p1\n";
  }
}

class D2 extends D1 {
  public function __construct(int $p1, int $p2) {
    parent::__construct($p1);
    echo "In D2 constructor, $p1, $p2\n";
  }
}

class D3 extends D2 {
  public function __construct(int $p1, int $p2, int $p3) {
    parent::__construct($p1, $p2);
    echo "In D3 constructor, $p1, $p2, $p3\n";
  }
}

class D4 extends D3 {
  public function __construct() {
    parent::__construct(1, 2, 3);
    echo "In D4 constructor\n";
  }
}

// ---------------------------------------------------------

// Play with private parameters
// starting with supposedly unintended uses

// *** private is accepted in a regular function; that doesn't sound right. Note also public/protected
// However, hhvm gags with Fatal error: syntax error, unexpected T_PRIVATE

//function f1(private int $p1, public int $p2, protected int $p3): void {}

// ---------------------------------------------------------

// can't have private in a closure's parameter list; okay

//function f2((function (private int, int, int): void) $p1): void {}

// ---------------------------------------------------------

function f3(int $iValue, (function (int): int) $process): int {
  return $process($iValue);
}

// *** private is accepted in an anonymous function; that doesn't sound right
// However, hhvm gags with Fatal error: syntax error, unexpected T_PRIVATE

function f4(): void {
//  $result = f3(5, function (private int $p) { return $p * 2; });	// doubles 5
}

// ---------------------------------------------------------

// *** private is accepted in an abstract method in an interface; that doesn't sound right
// However, hhvm gags with Fatal error: Parameters modifiers not allowed on methods

interface I1 {
//  public function f(private int $p1): void;
}

// ---------------------------------------------------------

// *** private is accepted in a method in a trait; that doesn't sound right
// However, hhvm gags with Fatal error: Parameters modifiers not allowed on methods

trait T1 {
//  public function f(private int $p1): void {}
}

// ---------------------------------------------------------

// *** private is accepted in a method in a class; that doesn't sound right
// However, hhvm gags with Fatal error: Parameters modifiers not allowed on methods

class C1 {
//  public function f(private int $p1): void {}
}

// ---------------------------------------------------------

// OK; auto generates 3 properties, $p2, $p4, and $p6, with the given type and visibility

class C2 {
  private int $pr1;
  protected int $pr3;
  public int $pr5;

  public function __construct(int $p1, private int $p2, int $p3, protected float $p4, int $p5,
    public string $p6 = "xxx") {
    $this->pr1 = $p1;
    $this->pr3 = $p3;
    $this->pr5 = $p5;
  }
}

// ---------------------------------------------------------

// If have explicit property name same as implicit one, get "Name already bound: px"; good
class C3 {
//  private ?int $p1;
//  private ?int $p2;
//  private ?int $p3;

  public function __construct(private int $p1, protected float $p2, public string $p3) {
  }
}

function main(): void {
//  $d1 = new D1(10);
//  $d2 = new D2(10, 20);
//  $d3 = new D3(10, 20, 30);
  $d4 = new D4();

  $c2 = new C2(10, 20, 30, 40.0, 50);
  var_dump($c2);
//  echo $c2->pr1 . "\n";	// can't access private member; good
//  echo $c2->p2 . "\n";	// can't access private member; good
//  echo $c2->pr3 . "\n";	// can't access protected member; good
//  echo $c2->p4 . "\n";	// can't access protected member; good
  echo $c2->pr5 . "\n";	// can access public member; good
  echo $c2->p6 . "\n";	// can access public member; good
}

/* HH_FIXME[1002] call to main in strict*/
main();
