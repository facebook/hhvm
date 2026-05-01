<?hh

// Calls the namespaced function — this is a production-affecting
// cross-package reference that should be detected.
function caller_of_ns_fun(): void {
  \BarNs\shared_fun();
}
