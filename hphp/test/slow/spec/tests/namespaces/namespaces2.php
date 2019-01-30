<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

namespace NS1
{
error_reporting(-1);

echo "Inside namespace " . __NAMESPACE__ . "\n";
echo "Inside function " . __FUNCTION__ . "\n";
echo "Inside method " . __METHOD__ . "\n";

//namespace NSX; //Cannot mix bracketed namespace declarations with unbracketed namespace declarations

//namespace NS2_nested {}   // Namespace declarations cannot be nested
}

//namespace NSY; //Cannot mix bracketed namespace declarations with unbracketed namespace declarations

namespace
{
echo "Inside namespace " . __NAMESPACE__ . "\n";
echo "Inside function " . __FUNCTION__ . "\n";
echo "Inside method " . __METHOD__ . "\n";
}


namespace NS2
{
echo "Inside namespace " . __NAMESPACE__ . "\n";
}
