<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

namespace NS1;          // define a level-1 namespace
echo "Inside namespace " . __NAMESPACE__ . "\n";

error_reporting(-1);


// namespace;   // cannot omit name (unlike namespace {)



namespace NS1\Sub1;     // define a level-2 namespace that happens to have a level-1 prefix
echo "Inside namespace " . __NAMESPACE__ . "\n";



namespace NS2;          // define a level-1 namespace
echo "Inside namespace " . __NAMESPACE__ . "\n";

use NS2;


namespace NS3\Sub1;     // define a level-2 namespace who's prefix is not an existing level-1 ns
echo "Inside namespace " . __NAMESPACE__ . "\n";

//class NS3\Sub1\C1     // prefix not allowed in definition
class C1
{
//  const NS3\Sub1\CON = 123;   // prefix not allowed in definition
    const CON = 123;

//  public function NS3\Sub1\f()    // prefix not allowed in definition
    public function f()
    {
        echo "Inside function " . __FUNCTION__ . "\n";
        echo "Inside method " . __METHOD__ . "\n";
    }
}

$c1 = new C1;
$c1->f();

Interface I1 {}
//Interface NS3\Sub1\I1 {}  // prefix not allowed in definition


/*
namespace NS10 {    // Cannot mix bracketed namespace declarations with unbracketed namespace declarations
    echo "Inside namespace " . __NAMESPACE__ . "\n";
}
*/
