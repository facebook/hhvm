<?hh // strict

namespace NS_constants;

function trace(string $name, mixed $value, bool $b = false): void {
  $r = define($name, $value, $b);
  echo "define $name " . ($r ? "succeeded" : "failed");
  if (defined($name))
    echo "; value is >" . constant($name)  . "<\n";
  else
    echo "; not defined\n";
}

function getValue(): int { return 250; }

class C {}

class MyClass {
//  define("DEFINE_INSIDE_CLASS", 10);		// not permitted inside a class; OK

//  const int CON30;		// not permitted; OK
  const float CON31 = 5 + 10.1 * 3;
//  const int CON34 = array(10, 20)[0];	// failed; Arrays are not allowed in class constants
//  const float CON37 = (float)10;// failed; unexpected '(float)'
//  const C CON40 = new C();	// failed; unexpected 'new'
  const int CON38 = 99;		// succeeded
  const int CON39 = MyClass::CON38;	// succeeded
}

class C3 {
  const CON1 = 123;		// implicitly static, and can't say so explicitly
//  public const CON2 = 123;	// all class constants are implicitly public; can't say explicitly
//  protected const CON3 = 123;	// all class constants are implicitly public
//  private const CON4 = 123;	// all class constants are implicitly public
}

function main(): void {
/*
// define some constants with simple (single-token) scalar initial values 

  trace("STATUS1", true);
  trace("MIN", 10);
  trace("MAX", 20, true);		// HHVM Warning: Case insensitive constant names are not supported in HipHop
  trace("MY_PI", 3.1415926);
  trace("MY_COLOR", "red");
  trace("C1", null);
*/

///*
// try to define some constants with multiple-token scalar initial values 

// involving literals only

  trace("CON1", 5 + 10.1 * 3);// succeeded;

// involving variables and other non-basic operators

  $v1 = 10;
  trace("CON2", 5 + $v1);			// succeeded
  $v2 = array(10, 20);
  trace("CON3", 5 + $v2[0]);		// succeeded
  trace("CON4", 5 + array(10, 20)[0]);	// succeeded
  trace("CON5", 5 + ++$v2[0]);		// succeeded
  trace("CON6", 5 + $v2[0]--);		// succeeded
  trace("CON7", 5 + (float)$v1);		// succeeded
  trace("CON8", 1 << $v1);		// succeeded
  trace("CON9", $v2[0] == $v2[1]);	// succeeded
  trace("CON10", $v2[0] = 123);		// succeeded
  trace("CON11", getValue() - 100);	// succeeded

// involving constants

  trace("CON21", 1 + constant("CON1"));		// succeeded;
  trace("CON22", 2 * constant("CON2") + constant("CON1"));	// succeeded;

//  define('X1', 10, false);
//  define('X2', 5 * X1, false);		// Unbound name: NS_constants\X1
//  define('X2', 5 * constant('X1'), false);
//  var_dump(constant('X1'), constant('X2'));

//*/

/*
// try to define some constants with names not permitted as tokens

  trace("36ABC", 100) . "\n";		// ill-formed name, but no error. Seems to work
  trace("#%&", 200) . "\n";		// ill-formed name, but no error. Seems to work
*/

/*
// try to redefine a user-defined constant

  trace("MY_COLOR", "green");		// Warning, and doesn't change the value

// try to redefine a pre-defined constant

  trace("true", 999) . "\n";		// HHVM:  Constant true already defined ...
  echo "    true's value:" . true . "\n";	// however, this shows the old value, 1
*/

/*
// try to define some constants with non-scalar initial values 

  trace("COLORS", [10, 20]);	// Constants may only evaluate to scalar values

  trace("MY_OBJECT", new C());	// Constants may only evaluate to scalar values

  $infile = fopen("Testfile.txt", 'r');
  if ($infile == false) {
    echo "Can't open file\n";
  }
  trace("MY_RESOURCE", $infile);	// HHVM: Constants may only evaluate to scalar values
*/

//  print_r(get_defined_constants());
}

/* HH_FIXME[1002] call to main in strict*/
main();
