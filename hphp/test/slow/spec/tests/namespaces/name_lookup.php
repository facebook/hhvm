<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

///*
function f1()
{
    echo "Inside function " . __FUNCTION__ . "\n";
}

f1();           // implicitly in current namespace
namespace\f1(); // explicitly in current namespace
\f1();          // explicitly in top-level scope
//*/

/*
namespace NS1;

function f2()
{
    echo "Inside function " . __FUNCTION__ . "\n";
}

f2();           // implicitly in current namespace
namespace\f2(); // explicitly in current namespace
\NS1\f2();      // explicitly in given namespace

//NS1\f2(); // looking for relative name NS1\NS1\f2(), which doesn't exist
//\namespace\f2();  // namespace keyword can only be a prefix
*/
