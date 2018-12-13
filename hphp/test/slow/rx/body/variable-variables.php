<?hh

<<__Rx>>
function test() {
  $ll = true;
  $v = __hhvm_intrinsics\launder_value('ll');
  $x = $$v;          // CGetN
  $x = $$v ?? false; // CGetQuietN
  $x = isset($$v);   // IssetN
  $x = empty($$v);   // EmptyN

  $$v = 1;           // SetN
  $$v *= 2;          // SetOpN
  $$v++;             // IncDecN
  unset($$v);        // UnsetN

  $ll = array('a' => 'b');
  $x = ${__hhvm_intrinsics\launder_value('ll')}['a']; // BaseNC
  $x = ($$v)['a'];                                    // BaseNL
}

<<__EntryPoint>>
function main() {
  test();
  echo "Done\n";
}
