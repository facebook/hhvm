<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

function msg()
{
    echo "In msg\n";
    return 999;
}

function f()
{
    for ($i = 1; $i < 3; ++$i)
    {
        try
        {
            throw new Exception;
        }
///*
        catch (Exception $e) {
            echo "In handler for Exception\n";
//          break;          // allowed
//          continue;       // allowed
//          goto end;       // allowed
//          return msg();   // expression is evaluated, but value actually returned
                            // when both returns exist is 20, from finally block,
        }
//*/
        finally
        {
            echo "In finally block\n";
//          break;          // not allowed
//          continue;       // not allowed
//          goto end;       // not allowed
//          return 20;      // if the catch-block is omitted, this returns control to the
                            // caller, and the exception currently uncaught is discarded.
        }
    }
end:
    return 1;
}

$r = f();
var_dump($r);
