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

  C::$p = array('a' => 'b');
  $x = C::${__hhvm_intrinsics\launder_value('p')}['a']; // BaseSC
  $l = 'p';
  $x = (C::$$l)['a'];                                   // BaseSL
}

<<__EntryPoint>>
function main() {
  test();
  echo "Done\n";
}
