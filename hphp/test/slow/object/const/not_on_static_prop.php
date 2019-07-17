<?hh

class C {
  <<__Const>> public static int $p = 0;
}

<<__EntryPoint>> function main() { echo "FAIL: this file should not parse\n"; }
