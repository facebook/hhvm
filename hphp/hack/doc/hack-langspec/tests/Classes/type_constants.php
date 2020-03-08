<?hh // strict

namespace NS_type_constants;

// ---------------------------------------------------------------------

// A "getting-acquainted" example

abstract class User {
  abstract const type T as arraykey;
  public function __construct(private this::T $id) {}
  public function getID(): this::T {
    return $this->id;
  }
}

trait UserTrait {
  require extends User;
}

interface IUser {
  require extends User;
}

class AppUser extends User implements IUser {
  const type T = int;
  use UserTrait;
}

class WebUser extends User implements IUser {
  const type T = string;
  use UserTrait;
}

class OtherUser extends User implements IUser {
  const type T = arraykey;
  use UserTrait;
}

/*
function main(): void {
  $au = new AppUser(-1);
  var_dump($au->getID());
  $wu = new WebUser('-1');
  var_dump($wu->getID());
  $ou1 = new OtherUser(-1);
  var_dump($ou1->getID());
  $ou2 = new OtherUser('-1');
  var_dump($ou2->getID());
}

main();
*/

// ---------------------------------------------------------------------

// type constants are implicitly public, but can't be explicitly declared as such

abstract class Ctc1 {
  abstract const type T;
//  abstract public const type T;
//  private abstract const type T;
//  protected abstract const type T;
}

// use public name Ctc1::T

function ftc1(Ctc1::T $p): Ctc1::T { return $p; }

// ---------------------------------------------------------------------

// Can have multiple type constants in a class, they can be interspersed with other members, and
// can have forward references to them (see T3 in __construct below).

abstract class Ctc2 {
  abstract const type T1;
  public function __construct(private this::T3 $id) {}
  abstract const type T2;
  public function getID(): this::T3 {
    return $this->id;
  }
  abstract const type T3;
}

// ---------------------------------------------------------------------

// Check the permitted orderings of the keywords

abstract class Ctc3a {
  abstract const type T1;	// OK
//  abstract type const T2;	// ill-formed
//  const abstract type T3;	// ill-formed
//  type abstract const T4;	// ill-formed
//  const type abstract T5;	// ill-formed
//  type const abstract T6;	// ill-formed
}

class Ctc3b {
  const type T1 = int;			// OK
  const type T2 as arraykey = int;	// OK
//  type const T3 = int;		// ill-formed
//  type const T4 as arraykey = int;	// ill-formed
}

// ---------------------------------------------------------------------

// an abstract class

abstract class Ctc4 {
  abstract const type T1;			// constraint omitted; OK
  abstract const type T2 as arraykey;		// constraint present; OK
//  abstract const type T3 = int;		// Declared as abstract, so cannot have a concrete type
//  abstract const type T4 as arraykey = int;	// Declared as abstract, so cannot have a concrete type

//  const type T5;				// Not declared as abstract, so must have a concrete type
//  const type T6 as arraykey;			// Not declared as abstract, so must have a concrete type
  const type T7 = int;				// Not declared as abstract, has a concrete type; OK
  const type T8 as arraykey = int;		// Not declared as abstract, has a concrete type; OK

//  abstract const type T9, T10 as arraykey;	// A list is not permitted
}

// ---------------------------------------------------------------------

// a concrete class

class Ctc5 {
//  abstract const type T1;			// Can't have abstract type constant in a concrete class
//  abstract const type T2 as arraykey;		// Can't have abstract type constant in a concrete class
//  abstract const type T3 = int;		// Can't have abstract type constant in a concrete class
//  abstract const type T4 as arraykey = int;	// Can't have abstract type constant in a concrete class

//  const type T5;				// Not declared as abstract, so must have a concrete type
//  const type T6 as arraykey;			// Not declared as abstract, so must have a concrete type
  const type T7 = int;				// Not declared as abstract, has a concrete type; OK
  const type T8 as arraykey = int;		// Not declared as abstract, has a concrete type; OK
}

// ---------------------------------------------------------------------

// interfaces

// can have multiple type constants, in any order, just like classes
// but can't have a constraint on a concrete type constant

interface Itc1a {
  abstract const type T1 as arraykey;
  public function getID1(): this::T1;
  const type T4a = int;
//  const type T4b as arraykey = int; // Disallowed: An interface cannot contain a partially abstract type constant
}

interface Itc1b {
  abstract const type T2;
  public function getID2(this::T2 $p): void;
  abstract const type T3;
  const type T4c = string;
}

// an interface can refer to a type constant from one of its base interfaces

interface Itc1c extends Itc1b {
  public function f(this::T2 $p): void;
}

// can implement multiple interfaces getting abstract and/or concrete type constants from each

class Ctc6 implements Itc1a, Itc1b {
  const type T1 = int;
  public function __construct(private this::T1 $id) {}
  public function getID1(): this::T1 {
    return $this->id;
  }

  const type T2 = string;
  public function getID2(this::T2 $p): void {}

  const type T3 = float;

  public function f(this::T4a $p1, this::T4c $p2): void {}
}

// ---------------------------------------------------------------------

// traits; type constants are not permitted!

trait Ttc1 {
//  abstract const type T1;			// Cannot declare a constant in a trait
//  const type T2 = int;			// Cannot declare a constant in a trait
}

// ---------------------------------------------------------------------

// redeclaring a type constant

abstract class Ctc7a {
  abstract const type T1;
//  abstract const type T1;			// cannot redeclare in same class
  const type T7 = int;
//  const type T7 = int;			// cannot redeclare in same class
}

abstract class Ctc7b extends Ctc7a {
  abstract const type T1;			// redeclared exactly as was; OK
  const type T7 = int;				// redeclared exactly as was; OK
  const type T8 = num;
}

class Ctc7c extends Ctc7b {
  const type T1 = float;			// finally gets a concrete type; OK
//  const type T7 = num;			// num is inconsistent with int; error
//  const type T8 = int;			// int is inconsistent with num; error
}

interface Itc2a {
  abstract const type T1;
//  abstract const type T1;			// cannot redeclare in same interface
//***  const type T7 = int;
//  const type T7 = int;			// cannot redeclare in same interface
}

interface Itc2b extends Itc2a {
  abstract const type T1;			// redeclared exactly as was; OK
//****  const type T7 = int;				// redeclared exactly as was; OK
  const type T8 = num;
}

interface Itc2c extends Itc2b {
  const type T1 = float;			// finally gets a concrete type; OK
//  const type T7 = num;			// num is inconsistent with int; error
//  const type T8 = int;			// int is inconsistent with num; error
}

class Ctc7d extends Ctc7c implements Itc2a {
  const type T7 = int;				// redeclared exactly as was; OK
}

// ---------------------------------------------------------------------

// multiple aliases for the same type constant

interface Itc8 {
  const type T = int;
}

class Ctc8a implements Itc8 {}
class Ctc8b extends Ctc8a {}

function ftc8a(Itc8::T $p): void {}
function ftc8b(Ctc8a::T $p): void {}
function ftc8c(Ctc8b::T $p): void {}

function ftcx(Itc8::T $p1, Ctc8a::T $p2, Ctc8b::T $p3): void {
  ftc8a($p1); ftc8a($p2); ftc8a($p3);
  ftc8b($p1); ftc8b($p2); ftc8b($p3);
  ftc8c($p1); ftc8c($p2); ftc8c($p3);
}

//ftcx(5, 10, 15);

// ---------------------------------------------------------------------

// Issue #105

interface Ix {
  const type T7 = int;
}

interface Iy extends Ix {
//  const type T7 = int;	// HHVM error: Cannot override previously defined type constant Ix::T7
}

class Cw {
  const type T7 = int;
}

class Cx extends Cw {
  const type T7 = int;
}

/*
// HHVM Error: cannot inherit the type constant T7 from Ix,
// because it was previously inherited from Cx

class Cy extends Cx implements Ix {
}
*/

interface I1 {
  abstract const type T1;
}

interface I2 extends I1 {
  abstract const type T1;
}

abstract class AC1 {
  abstract const type T1;
}

abstract class AC2 extends AC1 implements I1, I2 {
  abstract const type T1;
}

// ---------------------------------------------------------------------

// use :: in type contexts rather than in expression contexts

interface Itc9a {
  abstract const type T0;
}

interface Itc9b extends Itc9a {
  abstract const type T1;
}

abstract class Ctc9 implements Itc9b {
  const type T2 = int;
  abstract public function ftc9(
	Itc9b::T1 $p1,
	this::T2 $p2): void;
  public static Ctc9::T2 $count = 0;
}

function f9a(array<Ctc9::T2, bool> $p): void {}

function f9b(array<Ctc9::T1> $p1, array<Itc9b::T0> $p2): void {}


// can chain (with multiple ::) and use "keywords" in identifier contexts

interface I10 {
  const type this = int;	// this is an identifier, NOT a keyword
}

class C10 {
  const type self = I10;	// self is an identifier, NOT a keyword
}

function test10(C10::self::this $x): void {}


// ---------------------------------------------------------------------

// Example of using this::T

abstract class CBase {
  abstract const type T;
  public function __construct(protected this::T $value) {}
}

class Cstring extends CBase {
  const type T = string;
  public function getString(): string {
    return $this->value;	// gets the string
  }
}

class Cint extends CBase {
  const type T = int;
  public function getInt(): int {
    return $this->value;	// gets the int
  }
}

function main(): void {
  var_dump((new Cstring('abc'))->getString());
  var_dump((new Cint(123))->getInt());
}

/* HH_FIXME[1002] call to main in strict*/
main();
