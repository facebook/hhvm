<?hh // strict

namespace NS_classname;

/* --------------------------------------

Can use as an identifier

*/

//function classname(): void {}			// can be used as a function name

class C0 {
//  public function classname(): void {}	// can be used as a method name
//  const type classname = int;			// can be used as a type constant name
  const type classname = classname<C0>;		// first is an identifier, second is a keyword
}

function f0a(C0::classname $p): C0::classname { return $p; }
function f0b(classname<C0::classname> $p): void { var_dump($p); }

/* --------------------------------------

Use as a keyword in all the usual (type-specifier) places

*/

type TC0B = classname<C0>;

class C0B {
  public static classname<\Exception> $p101 = \Exception::class;
  public static classname<\NS_classname\C0> $p102 = \NS_classname\C0::class;
  public static classname<C0> $p1 = C0::class;
  public static function sf0(): void { var_dump(C0B::$p101, C0B::$p102, C0B::$p1); }

  const classname<C0> P0 = C0::class;
  public static string $str = 'NS_classname\C0';
/***/  public static function sf1(): void { echo "sf1: " . ((C0B::$str === C0B::$p1) ? 'TRUE' : 'FALSE') . "\n"; }	// surprise! accepted
  public static function sf2(): void { echo "sf2: " . ((C0B::$str == C0B::$p1) ? 'TRUE' : 'FALSE') . "\n"; }

  public static function f(classname<C0> $p) : classname<C0> { return C0::class; }
  const type Tp2 = classname<C0>;
  public static array<classname<C0>> $p2 = array(C0::class);
  public static array<string, classname<C0>> $p3 = array("xx" => C0::class);
  public static (classname<C0>, array<int>, float) $p4 = tuple(C0::class, array(99, 88, 77), 10.5);
  public static shape('x' => classname<C0>, 'y' => int) $p5 = shape('x' => C0::class, 'y' => 20);
  public static ?classname<C0> $p6 = C0::class;
}

class Cj1<T as classname<C0>> {}
class Cj2 extends Cj1<classname<C0>> {}

// --------------------------------------

abstract class C1 {
  public abstract function __construct();
  public static function hello(): void { echo "Inside method " . __METHOD__ . "\n"; }
}

class C2 extends C1 {
  public function __construct() {}
  public static function hello(): void { echo "Inside method " . __METHOD__ . "\n"; }
}

class C3 extends C2 {
  public function __construct() { parent::__construct(); }
  public static function hello(): void { echo "Inside method " . __METHOD__ . "\n"; }
}

function test2(classname<C2> $clsname): void {
  var_dump($clsname);

// implicit and explicit conversion to string

  echo "\$classname is >" . $clsname . "<\n";
  echo "\$classname is >" . (string)$clsname . "<\n";

//  $v = $clsname;
//  var_dump($v);

// use classname<T> with new

  $w = new $clsname();
//  $w = new '\NS_classname\C2'();	// Cannot use dynamic new in strict mode
//  $w = new $v();			// this works!

// use classname<T> with instanceof

  var_dump(new C2() instanceof $clsname);
  var_dump(new C3() instanceof $clsname);

// use classname<T> with scope-resolution operator ::

  $clsname::hello();

// However, can't use classname<T> with ::class

//  var_dump($clsname::class);	// invalid combo

///******/  var_dump(new C2() instanceof C1::class);	// checker allows, but HHVM doesn't
} 

function test3(classname<C3> $clsname): void {
  var_dump($clsname);
} 

class C4 {
  public array<classname<C1>> $vals1 = array(C1::class, C2::class, C3::class);
  public array<classname<I5>> $vals2 = array(I5::class, C5a::class, C5b::class);
} 

<<__ConsistentConstruct>>
interface I5 {
  static function hello(): void;
}

class C5a implements I5 {
  public static function hello(): void { echo "Inside method " . __METHOD__ . "\n"; }
}

class C5b implements I5 {
  public static function hello(): void { echo "Inside method " . __METHOD__ . "\n"; }
}

function test5(classname<I5> $clsname): void {
  var_dump($clsname);

// implicit and explicit conversion to string

  echo "\$classname is >" . $clsname . "<\n";
  echo "\$classname is >" . (string)$clsname . "<\n";

// use classname<T> with new

  $w = new $clsname();

// use classname<T> with instanceof

  var_dump(new C5a() instanceof $clsname);
  var_dump(new C5b() instanceof $clsname);

// use classname<T> with scope-resolution operator ::

  $clsname::hello();
} 

// --------------------------------------

// class has to be have an abstract or final constructor or be marked with the attribute <<__ConsistentConstruct>>

abstract class C8a {
  abstract public function __construct(); 
}

class C8b {
  final public function __construct() {} 
}

<<__ConsistentConstruct>>
class C8c {
}

function f8a(classname<C8a> $p): void {
  $v = new $p();
}

function f8b(classname<C8b> $p): void {
  $v = new $p();
}

function f8c(classname<C8c> $p): void {
  $v = new $p();
}

// --------------------------------------

function main(): void {

  echo "---------- sf0() --------------\n\n";

  C0B::sf0();

  echo "---------- sf1() --------------\n\n";

  C0B::sf1();

  echo "---------- sf2() --------------\n\n";

  C0B::sf2();

/*
  echo "---------- testf0() --------------\n\n";

  testf0(); 
*/

  echo "---------- var_dump --------------\n\n";

  var_dump(new C4()); 

  echo "\n---------- test2(C2::class) --------------\n\n";

  test2(C2::class);

  echo "\n---------- test3(C3::class) --------------\n\n";

  test3(C3::class);

  echo "\n---------- test2(C3::class) --------------\n\n";

  test2(C3::class);	// treats as a C3 rather than a C2; good

//  test3(C2::class);	// incompatible type

  echo "\n---------- test5(C5a::class) --------------\n\n";

  test5(C5a::class);

  echo "\n---------- test5(C5b::class) --------------\n\n";

  test5(C5b::class);

/*
  echo "\n---------- test5(I5::class) --------------\n\n";

  test5(I5::class);	// runtime error; can't use new with an interface name
*/
  echo "\n---------- end of script --------------\n\n";
}

/* HH_FIXME[1002] call to main in strict*/
main();
