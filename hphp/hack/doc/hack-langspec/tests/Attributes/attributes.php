<?hh // strict

namespace NS_attrib1;

// --------------------------------------------------------

// example for spec

<<Help("http://www.MyOnlineDocs.com/Widget.html")>>
class Widget {
  // ...
}

// --------------------------------------------------------

<<Attr1, Attr2(3, true, 3.4, 'top-level function')>>
function f1(): void { echo "Inside " . __FUNCTION__ . "\n"; }

// --------------------------------------------------------

// attributes accepted, but don't seem to serve a purpose

<<Attr1b, Attr2b(3, true, 3.4, 'trait')>>
trait T {
  <<Attr1, Attr2(3, true, 3.4, 'trait instance method')>>
  public function compute(): void { }

  <<Attr1, Attr2(3, true, 3.4, 'trait static method')>>
  public static function getData(): void { }
}

// --------------------------------------------------------

// attributes accepted, but don't seem to serve a purpose

<<Attr1c, Attr2c(3, true, 3.4, 'interface')>>
interface I {
  // attributes accepted, but don't seem to serve a purpose

  <<Attr1, Attr2(3, true, 3.4, 'interface instance method')>>
  public function f(): void;
}

// --------------------------------------------------------

// Check that token and whitespace handling is regular. It is
// Also note that an empty value list is equivalent to no value list

<<  Attr1()   , Attr2  (  3  , true  , 3.4  ,
	 'class C1'  )  >>

class C1 implements I {
  use T;

// hh_client - Expected modifier (Parsing[1002]); The class member _ is not always properly initialized
//
//  <<Attr1, Attr2(3, true, 3.4, 'const')>>
  const int CON = 100;

// hhvm - Fatal error: syntax error, unexpected T_STRING, expecting T_FUNCTION 
//
//  <<Attr1, Attr2(3, true, 3.4, 'static int property')>>
  private static int $pr1 = 0;

// hhvm - Fatal error: syntax error, unexpected T_STRING, expecting T_FUNCTION
//
//  <<Attr1, Attr2(3, true, 3.4, 'int property')>>
  private int $pr2 = 0;

  <<Attr1, Attr2(3, true, 3.4, 'constructor')>>
  public function __construct() { }

  <<Attr1, Attr2(3, true, 3.4, 'destructor')>>
  public function __destruct() { }

  <<Attr1, Attr2(3, true, 3.4, 'instance method')>>
  public function f(): void { }

// attributes on parameter accepted, but don't seem to serve a purpose

  public function f2(<<X>> int $p1): void { echo "Inside " . __FUNCTION__ . "\n"; }

  <<Attr1, Attr2(3, true, 3.4, 'static method')>>
  public static function sf(): void { }
}

// --------------------------------------------------------

// can't have duplicate attributes, with or without arguments
// Accepted by hh_client, but rejected by hhvm; Redeclared attribute Attr3

//<<Attr3, Attr3(3.4, 'class C2')>> 
//<<Attr3, Attr3>>
//<<Attr3(3.4, 'class C2'), Attr3(3.4, 'class C2')>>
class C2 {}

// --------------------------------------------------------

// Can a target have multiple attribute specifications? No

<<Attr4>>
//<<Attr5>>	// hh_client: rejected; Expected expression
class C3 {}

// --------------------------------------------------------

// What types can an attribute value have?

enum ControlStatus: int {
  Stopped = 0;
  Stopping = 1;
  Starting = 2;
  Started = 3;
}

// no errors from hh_client, but from hhvm, 
// User-defined constants are not allowed in user attribute expressions

class X {
  const int MAX = 100;
}

<<Attr6(true, 123, 3.4, 'class C4', null
//	, X::MAX			// disallowed
//	, STDERR			// disallowed
	, array(10, 20, 30)
	, array('x' => 3, 'y' => 5)
//	, ControlStatus::Stopping	// disallowed
	, tuple (12, 5.9)
	, shape('x' => -3, 'y' => 6)
//	, Vector {22, 33}		// disallowed
)>> 
class C4 {
  <<Attr6(true, 123, 3.4, 'method C4->f', null
//		, X::MAX			// disallowed
//		, STDERR			// disallowed
		, array(10, 20, 30)
		, array('x' => 3, 'y' => 5)
//		, ControlStatus::Stopping	// disallowed
		, tuple (12, 5.9)
		, shape('x' => -3, 'y' => 6)
//		, Vector {22, 33}		// disallowed
  )>> 
  public function f(): void { }
}

// --------------------------------------------------------

// attributes cannot go in a closure type nor on an anonymous function

/*
class C5 {
  private (<<XX>> function (): void) $prop;	// rejected by hh_client
  public function __construct() {
    $this->prop = <<XX>> function (): void { echo "Hi there!\n"; };	// rejected by hh_client
  }
}
*/

// --------------------------------------------------------

function main(): void {
  echo "\n============== top-level function =====================\n\n";

  f1();
  $rf = new \ReflectionFunction('\NS_attrib1\f1');
  $attr1 = $rf->getAttribute('Attr1');
  var_dump($attr1);
  $attr2 = $rf->getAttribute('Attr2');
  var_dump($attr2);

// The method getAttributes is undefined in an object of type ReflectionFunction

//  $attrs = $rf->getAttributes();
//  var_dump($attrs);

// --------------------------------------------------------

  $rc = new \ReflectionClass('\NS_attrib1\Widget');

  echo "\n============== class Widget =====================\n\n";

  $attrHelp = $rc->getAttribute('Help');
  var_dump($attrHelp);
//  $attrHelp = $rc->getAttributes();	// method not defined

  echo "\n============== class C1->f2 =====================\n\n";

  $c1 = new C1();
  $c1->f2(10);

  $rc1 = new \ReflectionClass('\NS_attrib1\C1');

  echo "\n============== class =====================\n\n";

  $attr1 = $rc1->getAttribute('Attr1');
  var_dump($attr1);
  $attr2 = $rc1->getAttribute('Attr2');
  var_dump($attr2);

  echo "\n============== constructor =====================\n\n";

  $attr1 = $rc1->getMethod('__construct')->getAttribute('Attr1');
  var_dump($attr1);
  $attr2 = $rc1->getMethod('__construct')->getAttribute('Attr2');
  var_dump($attr2);

  echo "\n============== destructor =====================\n\n";

  $attr1 = $rc1->getMethod('__destruct')->getAttribute('Attr1');
  var_dump($attr1);
  $attr2 = $rc1->getMethod('__destruct')->getAttribute('Attr2');
  var_dump($attr2);

  echo "\n============== instance method =====================\n\n";

  $attr1 = $rc1->getMethod('f')->getAttribute('Attr1');
  var_dump($attr1);
  $attr2 = $rc1->getMethod('f')->getAttribute('Attr2');
  var_dump($attr2);

  echo "\n============== static method =====================\n\n";

  $attr1 = $rc1->getMethod('sf')->getAttribute('Attr1');
  var_dump($attr1);
  $attr2 = $rc1->getMethod('sf')->getAttribute('Attr2');
  var_dump($attr2);

  echo "\n============== trait instance method =====================\n\n";

  $attr1 = $rc1->getMethod('compute')->getAttribute('Attr1');
  var_dump($attr1);
  $attr2 = $rc1->getMethod('compute')->getAttribute('Attr2');
  var_dump($attr2);

  echo "\n============== trait static method =====================\n\n";

  $attr1 = $rc1->getMethod('getData')->getAttribute('Attr1');
  var_dump($attr1);
  $attr2 = $rc1->getMethod('getData')->getAttribute('Attr2');
  var_dump($attr2);

  echo "\n============== class C4 =====================\n\n";

  $rc4 = new \ReflectionClass('\NS_attrib1\C4');
  $attr6 = $rc4->getAttribute('Attr6');
  var_dump($attr6);
  $attr6 = $rc4->getMethod('f')->getAttribute('Attr6');
  var_dump($attr6);
}

/* HH_FIXME[1002] call to main in strict*/
main();
