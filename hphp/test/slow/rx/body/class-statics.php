<?hh

class C { public static $p; }

<<__Rx>>
function test() {
  $x = C::$p;        // CGetS
                     // CGetQuietS does not exist
  $x = isset(C::$p); // IssetS
  $x = empty(C::$p); // EmptyS

  C::$p = 1;         // SetS
  C::$p *= 2;        // SetOpS
  C::$p++;           // IncDecS
                     // UnsetS does not exist
}

<<__EntryPoint>>
function main() {
  test();
  echo "Done\n";
}
