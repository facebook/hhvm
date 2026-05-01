<?hh

// Calls only the global shared_fun — NOT the namespaced \BarNs\shared_fun.
// Should NOT appear in package-lint results for targets_override_ns.php.
function caller_of_global_fun(): void {
  shared_fun();
}
