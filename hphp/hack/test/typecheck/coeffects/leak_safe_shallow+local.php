<?hh

function f_write_props()[write_props]: void {}

function f_globals()[globals]: void {}

function f_leak_safe()[leak_safe]: void {}

function f_leak_safe_bad()[leak_safe]: void {
    f_leak_safe_local();  // ERROR: missing SystemLocal
}

function f_zoned()[zoned]: void {
    f_leak_safe();  // OK
}
function f_zoned_shallow()[zoned_shallow]: void {
    f_leak_safe_shallow();  // OK
}
function f_zoned_local()[zoned_local]: void {
    f_leak_safe_local();  // OK
}

function f_leak_safe_shallow()[leak_safe_shallow]: void {
    f_leak_safe();   // OK
    f_write_props(); // OK
}

function f_leak_safe_shallow_bad()[leak_safe_shallow]: void {
    f_zoned_shallow();  // ERROR: missing ImplicitPolicyShallow FIXME
    f_globals();        // ERROR: missing AccessGlobals
}

function f_leak_safe_local()[leak_safe_local]: void {
    f_zoned_local();  // OK
    f_defaults();     // OK
}

function f_defaults(): void {}

function f_leak_safe_globals_bad()[write_props, globals]: void {
    f_leak_safe_shallow();  // ERROR: missing SystemShallow
}
