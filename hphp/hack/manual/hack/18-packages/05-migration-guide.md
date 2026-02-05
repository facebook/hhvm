# Migration Guide: Cross-Package Code Migration

This guide covers two common migration scenarios:

1. **Moving files between packages** (e.g., prod → intern)
2. **Restricting function access** with `__RequirePackage`

Both workflows use similar techniques: the typechecker catches static
violations, while runtime logging identifies dynamic callers.

The following assumes the following PACKAGES.toml:

```toml
[packages]

[packages.production]
include_paths = ["<some prod paths>"]
soft_includes = ["dynamically_accessible_in_production"]

[packages.intern]
include_paths = ["<some intern paths>"]
includes = ["production", "dynamically_accessible_in_production"]

[packages.dynamically_accessible_in_production]
includes = ["intern", "production"]

[deployments]

[deployments.production]
packages = ["production"]
soft_packages = ["dynamically_accessible_in_production"]

[deployments.intern]
packages = ["intern", "production", "dynamically_accessible_in_production"]
```

## Moving Files Between Packages

### How It Works

The `dynamically_accessible_in_production` packages allows files to be "in
transition" - accessible at runtime from production while the typechecker
enforces the shifted boundary.

### Steps

**1. `__PackageOverride`**

```hack no-extract
<?hh
// File is in an intern code-path such that by default it would be in the intern package
<<file: __PackageOverride('dynamically_accessible_in_production')>>

class ShouldBeInternOnlyFeature {}
```

**2. Fix static references from prod**

The typechecker will report errors. Fix them using, e.g., package expressions:

```hack no-extract
// within the production package
public function getFeatures(): vec<Feature> {
  $features = vec[/* prod features */];
  if (package intern) {
    $features[] = new ShouldBeInternOnlyFeature();
  }
  return $features;
}
```

**3. Deploy, monitor, and clean up**

Check the relevant logs for dynamic references. Once clean, simply remove the
`__PackageOverride`.

## Restricting Function Access with `__RequirePackage`

Use this workflow when one-or-more functions, rather than the entire file,
should be called only from a specific package.

### When to Use `__SoftRequirePackage`

Use `__SoftRequirePackage` as an intermediate step when you want to use
`__RequirePackage`, but you're concerned about dynamic callers (e.g.,
`meth_caller`, reflection) such that you want to identify all callers before
enforcing a hard requirement

If you're confident all callers are statically known, you may skip directly to
`__RequirePackage`. Just know that if you're wrong, the mistake will result in a
fatal.

### Steps

**1. Add `__SoftRequirePackage`**

```hack no-extract
<<__SoftRequirePackage('intern')>>
function do_intern_things(): void {
  // Typechecker now requires callers to have intern access
  // Runtime LOGS (doesn't fatal) if called without intern
}
```

**Note:** You cannot access symbols from the softly required package (intern, in
this case) in the method body.

**2. Fix static call sites**

The typechecker reports errors for call sites lacking package access. Three
options:

```hack no-extract
// Option A: Propagate the requirement
<<__SoftRequirePackage('intern')>>
function caller(): void {
  do_intern_things(); // OK
}

// Option B: Use package conditional
function caller(): void {
  if (package intern) {
    do_intern_things(); // OK
  }
}

// Option C: Move the caller into the intern package, potentially via the above migration guide
```

**3. Deploy and monitor**

For high-volume functions, use sampling to reduce log noise:

```hack no-extract
<<__SoftRequirePackage('intern', 1000)>> // Log 1 in 1000 calls
function do_intern_things(): void { ... }
```

Check the logs for unexpected callers.

**4. Fix dynamic callers and promote**

Once logs are clean, promote to the hard requirement:

```hack no-extract
<<__RequirePackage('intern')>>
function do_intern_things(): void {
  intern_helper(); // OK: can now access intern symbols
}
```

### Quick Reference

| Aspect                             | No Attribute | `__SoftRequirePackage` | `__RequirePackage` |
| ---------------------------------- | ------------ | ---------------------- | ------------------ |
| **Use Case**                       | Default      | Migration              | Final state        |
| **Typechecker Caller Enforcement** | ❌           | ✅                     | ✅                 |
| **Access to Required Package**     | ❌           | ❌                     | ✅                 |
| **Runtime Behavior**               | Default      | Logs                   | Asserts            |
