<?hh

function callee()[controlled]: void {}

function callable_from_defaults()[defaults]: void {
  callee();
}

function callable_from_unannotated(): void {
  callee();
}

function callable_from_policied()[policied]: void {
  callee();
}

class MyP {}

function callable_from_policied_of()[policied_of<\MyP>]: void {
  callee();
}

function uncallable_from_write_props_and_globals()[
  write_props, globals
]: void {
  callee(); // ERROR (missing a dedicated capability)
}
