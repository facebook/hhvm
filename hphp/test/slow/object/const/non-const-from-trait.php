<?hh

trait T { public int $p = 1; }
<<__Const>> class C { use T; }

<<__EntryPoint>> function main() { echo "FAIL: this file should not load\n"; }
