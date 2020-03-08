<?hh // strict

namespace NS1;			// define a level-1 namespace

function f1(): void {
  echo "Inside function " . __METHOD__. ", namespace " . __NAMESPACE__ . "\n";
}

namespace NS1\Sub1;		// define a level-2 namespace that happens to have a level-1 prefix

function f2(): void {
  echo "Inside function " . __METHOD__. ", namespace " . __NAMESPACE__ . "\n";
}

namespace NS2;			// define a level-1 namespace

function f3(): void {
  echo "Inside function " . __METHOD__. ", namespace " . __NAMESPACE__ . "\n";
}

use NS2;

namespace NS3\Sub1;		// define a level-2 namespace who's prefix is not an existing level-1 ns

function f4(): void {
  echo "Inside function " . __METHOD__. ", namespace " . __NAMESPACE__ . "\n";
}

class C1 {
  const CON = 123;

  public function f(): void {
    echo "Inside function " . __METHOD__. ", namespace " . __NAMESPACE__ . "\n";
  }
}

interface I1 {}

function f5(): void {
  echo "Inside function " . __METHOD__. ", namespace " . __NAMESPACE__ . "\n";
  $c1 = new C1();
  $c1->f();
}

function main(): void {
  \NS1\f1();
  \NS1\Sub1\f2();
  \NS2\f3();
  \NS3\Sub1\f4();
  \NS3\Sub1\f5();
}

/* HH_FIXME[1002] call to main in strict*/
main();
