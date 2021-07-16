<?hh

function call_controlled(
  (function()[controlled]: void) $f
)[controlled]: void {
  $f();
}

function good_fun_subtyping(): void {
  $l_controlled = ()[controlled] ==> {};
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
  call_controlled(() ==> {}); // ERROR: `controlled` is purer
  call_controlled(()[defaults] ==> {}); // ERROR: `controlled` is purer

  $l_controlled_and_write_globals = ()[controlled, globals] ==> {};
  call_write_props_and_globals($l_controlled_and_write_globals); // ERROR
  // (`controlled` should allow reads but not writes to globals )
}
