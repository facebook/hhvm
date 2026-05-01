<?hh

// References Foo_override_class via nameof and ::class only.
// These are non-production-affecting references (affects_prod_build = false)
// and should NOT count for --package-lint-full purposes.

function nonprod_nameof(): string {
  return nameof Foo_override_class;
}

function nonprod_class_ptr(): void {
  $_ = Foo_override_class::class;
}
