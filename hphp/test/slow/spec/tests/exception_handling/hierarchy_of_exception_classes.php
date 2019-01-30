<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

class DeviceException extends Exception { /*...*/ }
class DiskException extends DeviceException { /*...*/ }
class RemovableDiskException extends DiskException { /*...*/ }
class FloppyDiskException extends RemovableDiskException { /*...*/ }

function process()
{
    throw new DeviceException;
//  throw new Exception;
}

try
{
    process(); // call a function that might generate disk-related exception
}
/*

// see if compiler complains (ala C#) if there are unreachable catch clauses by putting
// the ultimate base exception class first. It does not.

catch (Exception $e) {
    echo "In handler for Exception\n";
    // ...
}
*/
catch (FloppyDiskException $fde)
{
    echo "In handler for FloppyDiskException\n";
    // ...
}
catch (RemovableDiskException $rde)
{
    echo "In handler for RemovableDiskException\n";
    // ...
}
catch (DiskException $de)
{
    echo "In handler for DiskException\n";
    // ...
}
catch (DeviceException $dve) {
    echo "In handler for DeviceException\n";
    // ...
}
///*
catch (Exception $e) {
    echo "In handler for Exception\n";
    // ...
}
//*/
finally
{
    echo "In finally block\n";
    // perform some cleanup
}
