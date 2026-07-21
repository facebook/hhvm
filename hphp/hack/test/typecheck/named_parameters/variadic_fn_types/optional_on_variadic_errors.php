<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

// `optional` on a variadic parameter (named or positional) is nonsensical —
// variadic parameters are already optional. Reject at parse time.

function f_optional_named_variadic(
  (function(optional named int...): void) $_,
): void {}

function f_optional_positional_variadic(
  (function(optional int...): void) $_,
): void {}
