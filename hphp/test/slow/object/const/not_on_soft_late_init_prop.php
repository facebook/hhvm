<?hh

class C {
  <<__Const, __SoftLateInit>> public int $p;
}

<<__EntryPoint>> function main() { echo "FAIL: this file should not parse\n"; }
