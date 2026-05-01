<?hh

// Namespaced definitions with __PackageOverride.
// Tests that target_names_of_ast uses fully-qualified NAST names
// and does not confuse \BarNs\shared_fun with \shared_fun.

<<file: __PackageOverride('foo')>>

namespace BarNs;

function shared_fun(): void {}
