<?hh // strict

namespace T;

function doit(): void { echo "Hi from doit\n"; }

namespace NS_generics;

// Type parameter names must be distinct within a given type parameter list

// You cannot re-bind the type parameter T
//class C1<T, T> {}
class C1<T1, T2> {}

// You cannot re-bind the type parameter T
//function f1<T, T>(): void {}
function f1<T1, T2>(): void {}

/*
If change any of the following 4 type names to start with T followed by a digit or upper/lowercase
letter only, checker thinks it looks like a generic type parameter, and suggests, "You probably forgot
to bind this type parameter right? Add <TK> somewhere (after the function name definition, or after the class name)
*/

enum TE2: int { T = 10; B = 30; }
class TC2 {}
interface TI2 {}
type TT2 = int;

class X<T> {
  const int T = 100;		// OK; T not used in a type context
  private ?array<TE2> $pr1;
  private ?array<TC2> $pr2;
  private ?array<TI2> $pr3;
  private ?array<TT2> $pr4;

  <<T>> public function T(): void { echo "Hi from T\n"; }	// OK; T as attribute and T as method
								// name not used in a type context

  public function f(): void {
    echo TE2::T . "\n";	// OK; T not used in a type context
    \T\doit();		// OK; T not used in a type context
  }

  // If a type parameter of g() is T, "You cannot re-bind the type parameter T"
//  public function g<T>(): void { }
//  public function g<Tx, T>(): void { }
  public function g<Tx>(): void { echo "Hi from g\n"; }
}

interface Y<T> {
  const int T = 100;			// OK; T not used in a type context
  <<T>> public function T(): void;	// OK; T as attribute and T as method
					// name not used in a type context

  // If a type parameter of g() is T, "You cannot re-bind the type parameter T"
//  public function g<T>(): void;
//  public function g<Tx, T>(): void;
  public function g<Tx>(): void;
}

trait Z<T> {
  private ?array<TE2> $pr1;
  private ?array<TC2> $pr2;
  private ?array<TI2> $pr3;
  private ?array<TT2> $pr4;

  <<T>> public function T(): void { echo "Hi from T\n"; }	// OK; T as attribute and T as method
								// name not used in a type context

  public function f(): void {
    echo TE2::T . "\n";	// OK; T not used in a type context
    \T\doit();		// OK; T not used in a type context
  }

  // If a type parameter of g() is T, "You cannot re-bind the type parameter T"
//  public function g<T>(): void { }
//  public function g<Tx, T>(): void { }
  public function g<Tx>(): void { echo "Hi from g\n"; }
}

class C2a {
  public function __construct<T>() { echo "In " . __METHOD__ . "\n"; }
  public function __destruct<T>() { echo "In " . __METHOD__ . "\n"; }
}

class C2b<T2> {
  public function __construct<T>() { echo "In " . __METHOD__ . "\n"; }
  public function __destruct<T>() { echo "In " . __METHOD__ . "\n"; }
}

class C2c<T2> {
// You cannot re-bind the type parameter T2
//  public function __construct<T2>() {}
//  public function __destruct<T2>() {}
}

function main(): void {
  $x = new X();
//  var_dump($x);
  var_dump(X::T);
  $x->T();
  $x->f();
  $x->g();

  $c2a = new C2a();
  $c2b = new C2b();
//  $c2b = new C2b<int>();
}

/* HH_FIXME[1002] call to main in strict*/
main();
