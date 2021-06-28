<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

interface i1 {}
interface i2 {}
class C1 {}
class C2 extends C1 implements i1, i2 {}
<<__EntryPoint>> function main(): void {
error_reporting(-1);

$c = new C2;
var_dump($c);
}
