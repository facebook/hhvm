<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/



interface XIterator extends Traversable
{
    function current():mixed;
    function key():mixed;
    function next():mixed;
    function rewind():mixed;
    function valid():mixed;
}

<<__EntryPoint>> function main(): void {
error_reporting(-1);
}
