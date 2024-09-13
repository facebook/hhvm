<?hh

function callee()[leak_safe]: void {}

function callable_from_defaults()[defaults]: void {
  callee();
}

function callable_from_unannotated(): void {
  callee();
}

function callable_from_policied()[zoned]: void {
  callee();
}

class MyP {}

function uncallable_from_write_props_and_globals()[
  write_props, globals
]: void {
  callee(); // ERROR (missing a dedicated capability)
}
