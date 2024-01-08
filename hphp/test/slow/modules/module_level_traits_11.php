<?hh

module MLT_B;

<<__NeverInline>>
function laundry_c_sbar(): mixed {
  return (__hhvm_intrinsics\launder_value("C::sbar"));
}

<<__NeverInline>>
function laundry_c_sfoo(): mixed {
  return (__hhvm_intrinsics\launder_value("C::sfoo"));
}

<<__NeverInline>>
function laundry_c(): mixed {
  $c = new C();
  return (__hhvm_intrinsics\launder_value($c));
}

function pp_exn(Exception $e) :mixed{
  return $e->getMessage() . " @ " . $e->getFile() . ":" . $e->getLine() . "\n";
}

<<__EntryPoint>>
function zot(): void {
  include 'module_level_traits_module_a.inc';
  include 'module_level_traits_module_b.inc';
  include 'module_level_traits_11.inc0';
  include 'module_level_traits_11.inc1';

  $c = laundry_c();
  try { $c->foo(); } catch (Exception $e) { echo pp_exn($e); }
  try { $c->bar(); } catch (Exception $e) { echo pp_exn($e); }

  $c_sfoo = laundry_c_sfoo();
  try { $c_sfoo(); } catch (Exception $e) { echo pp_exn($e); }

  $c_sbar = laundry_c_sbar();
  try { $c_sbar(); } catch (Exception $e) { echo pp_exn($e); }
}
