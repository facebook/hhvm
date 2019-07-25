<?hh

class C {
  <<__Const, __SoftLateInit>> static public int $p;
}

<<__EntryPoint>> function main() { echo "FAIL: this file should not parse\n"; }
