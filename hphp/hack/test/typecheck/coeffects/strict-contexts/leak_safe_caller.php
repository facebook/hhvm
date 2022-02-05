<?hh

function caller_good()[leak_safe]: void {
  f_read_globals(); // OK
  f_write_props();
}

function f_read_globals()[read_globals]: void {}
function f_write_props()[write_props]: void {}

function caller_bad()[leak_safe]: void {
  f_write_globals(); // ERROR: missing `AccessGlobals`

  f_policied(); // ERROR: missing `ImplicitPolicy`

  f_unannotated(); // ERROR
  f_defaults(); // ERROR
}

function f_write_globals()[globals]: void {}
function f_policied()[zoned]: void {}
function f_unannotated(): void {}
function f_defaults()[defaults]: void {}
