<?hh

class C { protected int $p = 1; }
<<__NoFlatten>> trait T { <<__Const>> public int $p = 2; }
class D extends C { use T; }

<<__EntryPoint>> function main() :mixed{ echo "FAIL: this file should not load\n"; }
