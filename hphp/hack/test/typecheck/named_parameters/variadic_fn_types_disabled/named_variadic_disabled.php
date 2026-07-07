<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

// With named_variadic_type=false, a named variadic in a function type is
// rejected at naming time.
function take_named_variadic((function(named int...): void) $_): void {}

function take_mixed((function(int, string..., named bool...): void) $_): void {}

// Regular variadic (positional) still works without the flag.
function take_regular((function(int...): void) $_): void {}
