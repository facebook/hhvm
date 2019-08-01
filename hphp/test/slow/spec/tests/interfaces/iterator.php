<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/



interface XIterator extends Traversable
{
    function current();
    function key();
    function next();
    function rewind();
    function valid();
}

<<__EntryPoint>> function main(): void {
error_reporting(-1);
}
