<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

// Unconditionally defined functions

ucf1();     // can call ucf1 before its definition is seen

function ucf1()
{
        echo "Inside unconditionally defined function ucf1\n";
}

ucf1();     // can call ucf1 after its definition is seen


// Conditionally defined functions

$flag = TRUE;

//cf1();    // Error; call to undefined function

if ($flag) {
    function cf1()
    {
        echo "Inside conditionally defined function cf1\n";
    }
}

if ($flag)
{
    cf1();  // can call cf1 now, as it's been defined
}
else
{
//  cf1();  // Error; call to undefined function
}

function ucf2()
{
    function cf2()
    {
        echo "Inside conditionally defined function cf2\n";
    }
}

//cf2();    // Error; call to undefined function
ucf2();     // cf2 now exists
cf2();      // Ok
