<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

// Tests for calling a value whose type has a named variadic parameter.

// Basic named-variadic-only function type: any number of named args
// with the right element type.
function call1((function(named int...): void) $f): void {
  $f();
  $f(a=1);
  $f(a=1, b=2, c=3);
}

// Positional + named variadic.
function call2((function(int, named string...): void) $f): void {
  $f(0);
  $f(0, label="x");
  $f(0, label="x", other="y");
}

// Required named + named variadic: the required must be supplied, the
// variadic extras are optional.
function call3((function(named int $x, named bool...): void) $f): void {
  $f(x=1);
  $f(x=1, on=true);
  $f(x=1, on=true, off=false);
}

// Element type is enforced on named-variadic arguments.
function call_err1((function(named int...): void) $f): void {
  $f(a="not an int"); // ERROR: string is not int
}

// Missing required positional still errors even with named variadic.
function call_err2((function(int, named string...): void) $f): void {
  $f(label="x"); // ERROR: missing positional
}

// Missing required named still errors even with named variadic.
function call_err3((function(named int $x, named bool...): void) $f): void {
  $f(on=true); // ERROR: missing required named x
}

// Duplicate named argument is still an error.
function call_err4((function(named int...): void) $f): void {
  $f(a=1, a=2); // ERROR: duplicate a
}
