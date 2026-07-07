<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

// Test that function types with named variadic parameters parse correctly
// in HHVM runtime (types are erased, but parsing should succeed)

function take_named_variadic((function(named int...): void) $f): void {
  // Function type hint with named variadic should parse
  var_dump("take_named_variadic called");
}

function take_mixed_variadic((function(int, string..., named bool...): void) $f): void {
  var_dump("take_mixed_variadic called");
}

function take_only_named_variadic((function(named string...): void) $f): void {
  var_dump("take_only_named_variadic called");
}

// Test with a simple function (no named variadic in definition,
// since that's not supported in HHVM runtime yet - only in type hints)
function test_fn(int $x): void {
  var_dump($x);
}

<<__EntryPoint>>
function main(): void {
  // Test that function type hints with named variadic parse and work at runtime
  // Note: function types are erased at runtime in HHVM, so this just tests parsing

  echo "OK - function types with named variadic parsed successfully\n";
}
