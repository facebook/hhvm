<?hh // strict

namespace NS_visibility;

class C {
// constants

  const int CON1 = 123;		// implicitly static, and can't say so explicitly
//  public const int CON2 = 123;	// class constants are implicitly public; can't say explicitly
//  protected const int CON3 = 123;	// class constants are implicitly public
//  private const int CON4 = 123;	// class constants are implicitly public

// properties

//  $prop1a;			// rejected; must have a modifier of some sort
//  $prop1b = 123;			// rejected; must have a modifier of some sort
  public int $prop2 = 0;
  protected int $prop3 = 0;
  private int $prop4 = 0;

//  public var $vprop1a;		// var not supported, with or without an explicit type
//  var $vprop1b = 123;		// 
//  public var $vprop2;		// can't combine var with visibility modifiers
//  var public  $vprop2;
//  protected var $vprop3;
//  var protected  $vprop3;
//  var private $vprop4;

  static int $sprop1 = 0;
  public static int $sprop2 = 0;
  static protected int $sprop3 = 0;	// visibility and static ordering unimportant
  private static int $sprop4 = 0;

//  static var $vsprop1;	// can't combine var with static modifier
//  var static $vsprop1;
//  var public static $vsprop2;
//  protected var static $vsprop3;
//  private static var $vsprop4;

// methods
	
  public function f1(): void {}
  public function f2(): void {}
  protected function f3(): void {}
  private function f4(): void {}

  static function sf1(): void {}
  public static function sf2(): void {}
  static protected function sf3(): void {}	// visibility and static ordering unimportant
  private static function sf4(): void {}

// constructors
	
  public function __construct() {}
//  protected function __construct() {}		// OK on its own
//  private function __construct() {}		// OK on its own

//  static function __construct() {}			// constructors can't be static
//  public static function __construct() {}
//  protected static function __construct() {}
//  private static function __construct() {}

// destructors
	
  public function __destruct() {}
//  protected function __destruct() {}		// OK on its own
//  private function __destruct() {}		// OK on its own

//  static function __destruct() {}			// destructors can't be static
//  public static function __destruct() {}
//  protected static function __destruct() {}
//  private static function __destruct() {}
}

abstract class D1 {
  public abstract function paf1(int $p1): void;
  abstract protected  function paf2(): void;
//  private abstract function paf3(): void;		// can't ever provide an implementation
  public static abstract  function pasf1(): void;
  protected abstract static function pasf2(int $p1): void;
}

class D2 extends D1 {
//  public function paf1(): void {}			// Declaration of D2::paf1() must be compatible with D1::paf1($p1)
  public function paf1(int $q1): void {}		// OK; has same visibility as abstract decl, and same signature
//  public function paf1(int $q1, int $q2): void {}	// Declaration of D2::paf1() must be compatible with D1::paf1($p1)
//  protected function paf1(int $q1): void {}		// Access level to D2::paf1() must be public
//  private function paf1(int $q1): void {}		// Access level to D2::paf1() must be public

//  public function paf2(): void {}			// OK; has wider visibility than abstract decl
  protected function paf2(): void {}		// OK; has same visibility as abstract decl
//  private function paf2(): void {}			// Access level to D2::paf2() must be protected

  public static function pasf1(): void {}	// OK; has same visibility as abstract decl
//  protected static function pasf1(): void {}// Access level to D2::pasf1() must be public
//  private static function pasf1(): void {}	// Access level to D2::pasf1() must be public

//  static public function pasf2(int $q1): void {}	// OK; has wider visibility than abstract decl
//  static protected function pasf2(): void {}	// Declaration of D2::pasf2() must be compatible with D1::pasf2($p1)
  static protected function pasf2(int $q1): void {}	// OK; has same visibility as abstract decl, and same signature
//  static protected function pasf2(int $q1, int $q2): void {}	// Declaration of D2::pasf2() must be compatible with D1::pasf2($p1)
//  static private function pasf2(int $q1): void {}	// Access level to D2::pasf2() must be protected
}

function main(): void {
  echo "CON1: " . C::CON1 . "\n";	// use :: notation, as a const is implicitly static

  $c = new C();	// calls public constructor
  var_dump($c->prop2);	// accesses public instance method
}

/* HH_FIXME[1002] call to main in strict*/
main();
