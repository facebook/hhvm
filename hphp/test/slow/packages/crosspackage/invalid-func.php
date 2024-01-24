<?hh

module a; // package foo

<<__EntryPoint, __CrossPackage("baz")>> // Error - package baz is not loaded by the same deployment as package foo
public function foo() : void {}
