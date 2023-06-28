<?hh

class C { <<__Const>> protected int $p = 1; }
class D extends C { public int $p = 2; }

<<__EntryPoint>> function main() :mixed{ echo "FAIL: this file should not load\n"; }
