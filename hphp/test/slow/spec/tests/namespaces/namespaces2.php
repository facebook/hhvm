<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

namespace NS1
{
   function foo() :mixed{
      echo "Inside namespace " . __NAMESPACE__ . "\n";
      echo "Inside function " . __FUNCTION__ . "\n";
      echo "Inside method " . __METHOD__ . "\n";
   }

//namespace NSX; //Cannot mix bracketed namespace declarations with unbracketed namespace declarations

//namespace NS2_nested {}   // Namespace declarations cannot be nested
}

//namespace NSY; //Cannot mix bracketed namespace declarations with unbracketed namespace declarations

namespace
{
   <<__EntryPoint>>
   function main() :mixed{
      \error_reporting(-1);

      \NS1\foo();

      echo "Inside namespace " . __NAMESPACE__ . "\n";
      echo "Inside function " . __FUNCTION__ . "\n";
      echo "Inside method " . __METHOD__ . "\n";

      \NS2\foo();
   }
}


namespace NS2
{
   function foo() :mixed{
      echo "Inside namespace " . __NAMESPACE__ . "\n";
      echo "Inside function " . __FUNCTION__ . "\n";
      echo "Inside method " . __METHOD__ . "\n";
   }
}
