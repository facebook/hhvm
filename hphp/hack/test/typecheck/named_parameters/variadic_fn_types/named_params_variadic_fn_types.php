<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

// Test named variadic parameters in function types
// Note: function TYPE syntax for variadic is: type ... [$name]
// NOT: type $name ...

// Basic named variadic function type
function take_named_variadic((function(named int...): void) $_): void {}

// Function type with positional variadic + named variadic
function take_mixed_variadic((function(int, string..., named bool...): void) $_): void {}

// Named variadic with no positional params
function take_only_named_variadic((function(named string...): void) $_): void {}

// Named variadic with regular named params before it
function take_named_plus_variadic((function(named int $n, named string...): void) $_): void {}

// Positional params + named params + named variadic
function take_full((function(int, string, named bool $b, named float...): void) $_): void {}

// Test that function types with named variadic can be used in type aliases
type NamedVariadicFn = (function(named int...): string);
type MixedVariadicFn = (function(float, string..., named bool...): void);

// Test subtyping: optional named variadic
function take_optional_named_variadic((function(optional named int...): void) $_): void {}

// Ensure regular variadic still works
function take_regular_variadic((function(int...): void) $_): void {}
function take_regular_variadic2((function(int, string...): void) $_): void {}

// Ensure named non-variadic still works
function take_named((function(named int $x, named string $y): void) $_): void {}
