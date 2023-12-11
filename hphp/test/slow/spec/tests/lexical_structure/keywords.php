<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/
<<__EntryPoint>> function main(): void {
error_reporting(-1);

$colors = vec["red", "white", "blue"];

foreach ($colors as $a)
{
    echo $a.' ';
}
}
