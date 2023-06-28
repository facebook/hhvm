<?hh

class C { protected int $p = 1; }
class D extends C { <<__Const>> public int $p = 2; }

<<__EntryPoint>> function main() :mixed{ echo "FAIL: this file should not load\n"; }
