<?hh

// Caller lives naturally in `bar`, but overrides itself into `foo`.
// It references Foo_override_class (defined in bar/targets_override.php,
// which also overrides itself into `foo`).  Because the caller is
// effectively in `foo` — and `foo` cannot access `bar` — removing the
// target's __PackageOverride would break this caller.  Therefore
// --package-lint-full must report this file: the target's override is
// NOT excessive while this caller exists.

<<file: __PackageOverride('foo')>>

function caller_with_override(): void {
  Foo_override_class::foo_override_static_method();
}
