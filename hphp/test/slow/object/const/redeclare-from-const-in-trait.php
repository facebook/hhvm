<?hh

class C { <<__Const>> protected int $p = 1; }
trait T { public int $p = 2; }
class D extends C { use T; }

<<__EntryPoint>> function main() :mixed{ echo "FAIL: this file should not load\n"; }
