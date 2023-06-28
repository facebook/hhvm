<?hh // strict

function byRef(inout $x) :mixed{}

function test() :mixed{
  $x = null;
  byRef(inout $x?->y); // error
}
