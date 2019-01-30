<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

// ---------------------------------------------------------------

function displayExceptionObject(Exception $e)
{
    echo "\$e = >$e<\n";        // calls __toString
    echo "getMessage:       >".$e->getMessage()."<\n";
    echo "getCode:          >".$e->getCode()."<\n";
    echo "getPrevious:      >".$e->getPrevious()."<\n";
    echo "getFile:          >".$e->getFile()."<\n";
    echo "getLine:          >".$e->getLine()."<\n";
    echo "getTraceAsString: >".$e->getTraceAsString()."<\n";

    $traceInfo = $e->getTrace();
    var_dump($traceInfo);
    echo "Trace Info:".((count($traceInfo) == 0) ? " none\n" : "\n");
    foreach ($traceInfo as $traceInfoKey => $traceLevel)    // process all traceback levels
    {
        echo "Key[$traceInfoKey]:\n";
        foreach ($traceLevel as $levelKey => $levelVal)     // process one traceback level
        {
            if ($levelKey != "args")
            {
                echo "  Key[$levelKey] => >$levelVal<\n";
            }
            else
            {
                echo "  Key[$levelKey]:\n";
                foreach ($levelVal as $argKey => $argVal)   // process all args for that level
                {
                    echo "    Key[$argKey] => >$argVal<\n";
                }
            }
        }
    }
}

// ---------------------------------------------------------------

$prev = set_exception_handler(NULL);    // set to default handler
var_dump($prev);

// define a default un-caught exception handler

function MyDefExHandler(Exception $e)
{
    echo "In MyDefExHandler\n";
    displayExceptionObject($e);
    echo "Leaving MyDefExHandler\n";
}

// establish a new un-caught exception handler

$prev = set_exception_handler("MyDefExHandler");    // use my handler
var_dump($prev);

// try it out

function f($p1, $p2)
{
try {
    echo "In try-block\n";

    throw new Exception("Watson, come here!", 1234);
}

// no catch block(s)

finally {
    echo "In finally-block\n";
}

echo "Beyond try/catch/finally blocks\n==========\n";
}

/*
restore_exception_handler();
*/

echo "About to call f\n";
f(10, TRUE);
echo "Beyond the call to f()\n";    // never gets here; script terminates after my handler ends
