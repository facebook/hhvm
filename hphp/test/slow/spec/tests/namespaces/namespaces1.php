<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

namespace NS1;          // define a level-1 namespace
function foo() :mixed{
  echo "Inside namespace " . __NAMESPACE__ . "\n";
}

// namespace;   // cannot omit name (unlike namespace {)



namespace NS1\Sub1;     // define a level-2 namespace that happens to have a level-1 prefix
function foo() :mixed{
  echo "Inside namespace " . __NAMESPACE__ . "\n";
}


namespace NS2;          // define a level-1 namespace
function foo() :mixed{
  echo "Inside namespace " . __NAMESPACE__ . "\n";
}

use NS2;


namespace NS3\Sub1;     // define a level-2 namespace who's prefix is not an existing level-1 ns
function foo() :mixed{
  echo "Inside namespace " . __NAMESPACE__ . "\n";
}

//class NS3\Sub1\C1     // prefix not allowed in definition
class C1
{
//  const NS3\Sub1\CON = 123;   // prefix not allowed in definition
    const CON = 123;

//  public function NS3\Sub1\f()    // prefix not allowed in definition
    public function f()
:mixed    {
        echo "Inside function " . __FUNCTION__ . "\n";
        echo "Inside method " . __METHOD__ . "\n";
    }
}

interface I1 {}
//Interface NS3\Sub1\I1 {}  // prefix not allowed in definition


/*
namespace NS10 {    // Cannot mix bracketed namespace declarations with unbracketed namespace declarations
    echo "Inside namespace " . __NAMESPACE__ . "\n";
}
*/
<<__EntryPoint>>
function entrypoint_namespaces1(): void {
  \NS1\foo();
  \NS1\Sub1\foo();
  \NS2\foo();
  \NS3\Sub1\foo();

  $c1 = new C1;
  $c1->f();
}
