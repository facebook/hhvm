<?hh

class C {}
<<__Const>> class D extends C {}

<<__EntryPoint>> function main() { echo "FAIL: this file should not load\n"; }
