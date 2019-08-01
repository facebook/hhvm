<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

interface XArrayAccess
{
    function offsetExists ($offset);
    function offsetGet ($offset);
    function offsetSet ($offset, $value);
    function offsetUnset ($offset);
}
<<__EntryPoint>> function main(): void {
error_reporting(-1);
}
