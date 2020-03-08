<?hh // strict

namespace NS_enum;

enum BitFlags: int as int {
  F2 = BitFlags::F1 << 1;
  F3 = BitFlags::F2 << 1;
  F1 = 1;
}

enum ControlStatus: int {
  Stopped = 0;
  Stopping = 1;
  Starting = 2;
  Started = 2 + 1;		// use an a non-trivial expression
}

enum ControlStatus2    : int as int {
  Stopped = 0;
  Stopping = 1;
  Starting = 2;
  Started = 4 - 1;		// use an a non-trivial expression
}

/*
// an enum must have an explicit underlying type

enum E1 {
}
*/

// an enum can be empty

enum E2a: int {
}

enum E2b: string {
}

/* mixed enums no longer allowed

enum E2c: mixed {
}
*/

/*
// an enum must have an explicit underlying type of int, string, or mixed

enum E2d: bool {
}

enum E2e: float {
}

enum E2f: num {
}
*/

/*
// an enum constant must be initialized explicitly; no default values are assigned

enum E3: int {
	X;	// need an = and initial value
}
*/

enum E4a: string {
  Ready = 'READY';
  Set = 'SET';
  Go = 'GO';
}

enum E4b: string as string {
  Ready = 'READY';
  Set = 'SET';
  Go = 'GO';
}

/* mixed enums no longer allowed

enum E5: mixed {
  Ready = 'R';
  Set = 'S';
  Stopped = 10;
  Started = 20;

//  Status = false;	// Enum constants must be an int or string
//  Value = 1.234;	// Enum constants must be an int or string
//  X = null;	// Enum constants must be an int or string
}
*/

// an int enum can have negative constants, duplicate values, and gaps in the sequence

enum E6a: int {
  val1 = 10;
  val2 = 20;
  val3 = -5;		// negative value okay
  val4 = 20;		// duplicate okay
//  val5 = val1;		// unbound name val1
//  val6 = E6a::val1;	// Constant of type int conflicts with type E6a
//  val7 = self::val1;	// Constant of type int conflicts with type E6a
}

enum E6c: int as int {
  val1 = 10;
  val2 = 20;
  val3 = -5;		// negative value okay
  val4 = 20;		// duplicate okay
}

// a string enum can have duplicate values

enum E6b: string {
  v1 = 'xx';
  v2 = 'xx';
}

/*
// a string enum with as int clause

enum E7a: string as int {	// Invalid constraint on enum
}

// an int enum with as string clause

enum E7b: int as string {	// Invalid constraint on enum
}

// a mixed enum with as string clause

enum E7c: mixed as string {	// Invalid constraint on enum
}

// a mixed enum with as int clause

enum E7d: mixed as int {	// Invalid constraint on enum
}
*/

// a mixed enum with as mixed clause

/* mixed enums no longer allowed

enum E7e: mixed as mixed {	// Allowed, but not obvious what the value of that might be
  con1 = 123;
}
*/

enum E8a: int {
 con1 = 0;
}

enum E8b: string {
  con1 = "";
}

enum Permission: string {
  Read = 'R';
  Write = 'W';
  Execute = 'E';
  Delete = 'D';
}

enum Months: int as int {
  January = 0;
  February = 1;
  March = 2;
  April = 3;
  May = 4;
  June = 5;
  July = 6;
  August = 7;
  September = 8;
  October = 9;
  November = 10;
  December = 11;
}

//function main(ControlStatus $p1, ControlStatus2 $p2, E4a $p3, E5 $p4, E6a $p5): void {
function main(ControlStatus $p1, ControlStatus2 $p2, E4a $p3, E6a $p5): void {
  var_dump(ControlStatus::Started);	// int

  echo "====== use an enum value to control a switch ======\n\n";

  switch ($p1) {
  case ControlStatus::Stopped:
    echo "Stopped: $p1\n";
    break;
  case ControlStatus::Stopping:
    echo "Stopping: $p1\n";
    break;
  case ControlStatus::Starting:
    echo "Starting: $p1\n";	// ***
    break;
  case ControlStatus::Started:
    echo "Started: $p1\n";
    break;
  }

  switch ($p3) {
  case E4a::Ready:
    echo "Ready: $p3\n";	// ***
    break;
  case E4a::Set:
    echo "Set: $p3\n";
    break;
  case E4a::Go:
    echo "Go: $p3\n";
    break;
  }

/*
// The following debug code was used to figure what was going wrong when E5::Ready was initialized
// with the value 0. Doing that caused $p4 (= 'R') to match the case label E5::Started, as 'R' == 0 
// tests True in PHP and so in hhvm. That is, switch/case matching appears to use == rather than ===.

  echo "++++++\n";
  var_dump($p4);	// 'R'
  echo $p4 . " == " . E5::Set . " is " . (($p4 == E5::Set) ? "True\n" : "False\n");		// False
  echo $p4 . " == " . E5::Started . " is " . (($p4 == E5::Started) ? "True\n" : "False\n");	// True **
  echo $p4 . " === " . E5::Started . " is " . (($p4 === E5::Started) ? "True\n" : "False\n");	// False
  echo $p4 . " == " . E5::Stopped . " is " . (($p4 == E5::Stopped) ? "True\n" : "False\n");	// False
  echo $p4 . " == " . E5::Ready . " is " . (($p4 == E5::Ready) ? "True\n" : "False\n");		// True
  echo $p4 . " === " . E5::Ready . " is " . (($p4 === E5::Ready) ? "True\n" : "False\n");	// True
  echo "++++++\n";

  switch ($p4) {
  case E5::Set:
    echo "Set: $p4\n";
    break;
  case E5::Started:
    echo "Started: $p4\n";
    break;
  case E5::Stopped:
    echo "Stopped: $p4\n";
    break;
  case E5::Ready:
    echo "Ready: $p4\n";
    break;
  }
*/
  echo "\n====== does an int enum behave like an int? Ordinarily, no. ======\n\n";

  if (ControlStatus::Started == 3) ;

//  if (ControlStatus::Started < 3) ;	// This is an object of type NS_enum\ControlStatus
					// It is incompatible with an int

//  $r = ControlStatus::Stopping << 3;	// This is an int because this is used in a bitwise
					// operation. It is incompatible with an object of type
					// NS_enum\ControlStatus

//  $r = ControlStatus::Stopping * 10;	// This is a num (int/float) because this is used in
					// an arithmetic operation. It is incompatible with
					// an object of type NS_enum\ControlStatus

  echo "\n====== does an int enum behave like an int? Can do if an \"as int\" clause is present in the definition. ======\n\n";

  if (ControlStatus2::Started == 3) ;
  if ($p2 < 3) ;
  $r = ControlStatus2::Stopping << 3;
  $r = $p2 * 10;

// Clearly, I can use the enum constant as an int, so why does the checker gag on the two 
// subscript attempts?

  Months::March < 6;
  Months::March << 3;
  Months::March * 10;

  $monthTotals = array(100, 200, 300, 100, 200, 300, 100, 200, 300, 100, 200, 300);
//  echo "Total for March is " . $monthTotals[Months::March] . "\n"; 

  $monthTotals = Vector {100, 200, 300, 100, 200, 300, 100, 200, 300, 100, 200, 300};
//  echo "Total for March is " . $monthTotals[Months::March] . "\n"; 

  echo "\n====== use enum having duplicate enum constant values ======\n\n";

  if ($p5 == E6a::val1) echo "\$p5 == E6a::val1\n";
  if ($p5 == E6a::val2) echo "\$p5 == E6a::val2\n";	// gets here
  if ($p5 == E6a::val3) echo "\$p5 == E6a::val3\n";
  if ($p5 == E6a::val4) echo "\$p5 == E6a::val4\n";	// and here too

  echo "\n====== use Enum static methods ======\n\n";

//  $e = new \Enum();	// Unbound name: Enum; hmm. I was expecting to get something like
				// "Can't instantiate an abstract class"

  $names = ControlStatus::getNames();
  echo "ControlStatus::getNames() ---\n";
  foreach ($names as $key => $value) {
    echo "\tkey >$key< has value >$value<\n";
  }

  $values = ControlStatus::getValues();
  echo "ControlStatus::getValues() ---\n";
  foreach ($values as $key => $value) {
    echo "\tkey >$key< has value >$value<\n";
  }
/*
  $names = E5::getNames();
  echo "E5::getNames() ---\n";
  foreach ($names as $key => $value) {
    echo "\tkey >$key< has value >$value<\n";
  }

  $values = E5::getValues();
  echo "E5::getValues() ---\n";
  foreach ($values as $key => $value) {
    echo "\tkey >$key< has value >$value<\n";
  }
*/
/*
// Due to duplicate enum constant values, the call to getNames results in
// Fatal error: Uncaught exception 'HH\InvariantException' with message 'Enum has overlapping values' ...

  $names = E6a::getNames();
  echo "E6a::getNames() ---\n";
  foreach ($names as $key => $value) {
    echo "\tkey >$key< has value >$value<\n";
  }
*/

// However, getValues works okay with dupe values

  $values = E6a::getValues();
  echo "E6a::getValues() ---\n";
  foreach ($values as $key => $value) {
    echo "\tkey >$key< has value >$value<\n";
  }
/*
  echo "\nValue 'R'         " . (E5::isValid('R') ? 'is' : 'is not') . " a value in the enum\n";
  echo "Value E5::Ready   " . (E5::isValid(E5::Ready) ? 'is' : 'is not') . " a value in the enum\n";
  echo "Value 1           " . (E5::isValid(1) ? 'is' : 'is not') . " a value in the enum\n";
  echo "Value E5::Stopped " . (E5::isValid(E5::Stopped) ? 'is' : 'is not') . " a value in the enum\n";
  echo "Value 7           " . (E5::isValid(7) ? 'is' : 'is not') . " a value in the enum\n";
  echo "Value 'X'         " . (E5::isValid('X') ? 'is' : 'is not') . " a value in the enum\n";
*/

  echo "\n====== string operations on a string enum constant  ======\n\n";

  var_dump('READY'[1]);	// outputs string(1) "E"

//  var_dump(E4a::Ready[1]);		// rejected, as expected
  echo "E4a:Ready is " . E4a::Ready . "\n";	// concat works for enum (special case, apparently)
//  takes_string(E4a::Ready);		// rejected, as expected

//  var_dump(E4b::Ready[1]);		// rejected, hmm; I expected this to work (Issue #60)
  echo "E4b:Ready is " . E4b::Ready . "\n";	// concat works for enum
  takes_string(E4b::Ready);		// acepted; enum behaves as string

  echo "\n====== int operations on an int enum constant  ======\n\n";

//  takes_int(ControlStatus::Starting);	// rejected, as expected
  takes_int(ControlStatus2::Starting);

/*
  echo "\n====== mixed operations on an mixed enum constant  ======\n\n";

  takes_mixed(E5::Stopped);		// acepted; but then mixed accepts any kind of argument
					// including one of enum type, so the mixed enum is NOT
					// being treated like a mixed
  takes_mixed(E7e::con1);		// acepted; enum behaves as string
*/
}

function takes_int(int $x): void { var_dump($x); }
function takes_string(string $x): void { var_dump($x); }
function takes_mixed(mixed $x): void { var_dump($x); }

//main(ControlStatus::Started, ControlStatus2::Started, E4a::Ready, E6a::val2);
