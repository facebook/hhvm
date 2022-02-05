<?hh

function call_controlled(
  (function()[leak_safe]: void) $f
)[leak_safe]: void {
  $f();
}

function good_fun_subtyping(): void {
  $l_controlled = ()[leak_safe] ==> {};
  call_controlled($l_controlled); // OK (exact type)

  $l_write_props_and_globals = ()[write_props, read_globals] ==> {};
  call_controlled($l_write_props_and_globals); // OK (proper subtype)
}

function call_write_props_and_globals(
  (function()[write_props, globals]: void) $f
)[write_props, globals]: void {
  $f();
}

function bad_fun_subtyping(): void {
  call_controlled(() ==> {}); // ERROR: `leak_safe` is purer
  call_controlled(()[defaults] ==> {}); // ERROR: `leak_safe` is purer

  $l_controlled_and_write_globals = ()[leak_safe, globals] ==> {};
  call_write_props_and_globals($l_controlled_and_write_globals); // ERROR
  // (`leak_safe` should allow reads but not writes to globals )
}
