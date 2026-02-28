# Deployments

## What is a Deployment?

A **deployment** is a set of packages built and deployed together. One may think of it as a build target configuration telling HHVM which code to compile and run.

**Key requirements:**
- A deployment must be **transitively closed** with respect to package dependencies
- If package A includes package B, and A is deployed, B must also be explicitly deployed

## Defining Deployments

Deployments are defined in the `[deployments]` section of `PACKAGES.toml`:

```toml PACKAGES.toml
[packages]

[packages.production]
include_paths = ["//flib/"]

[packages.test]
include_paths = ["//flib/test/"]
includes = ["production"]

[deployments]

[deployments.production]
packages = ["production"]

[deployments.test]
packages = ["test", "production"]  # Must include production since test depends on it
```

### Transitive Closure Requirement

The `packages` list must include all transitive dependencies:

```toml
[packages.core]
include_paths = ["//flib/core/"]

[packages.utils]
include_paths = ["//flib/utils/"]
includes = ["core"]

[packages.app]
include_paths = ["//flib/app/"]
includes = ["utils"]

[deployments.app]
# Error: Missing core
# packages = ["app", "utils"]

# Correct: Includes all transitive dependencies
packages = ["app", "utils", "core"]
```

Even though `app` only directly includes `utils`, the deployment must explicitly list all three packages.

## Using Deployments with HHVM

Specify the active deployment:

```ini
Eval.Runtime.ActiveDeployment = production
```

When running in [repo authoritative](/hhvm/advanced-usage/repo-authoritative) mode, HHVM only compiles and runs files from packages in the specified deployment. References to undeployed packages cause runtime errors (e.g., "Class not found" or "Call to undefined function"). When running in other modes, the active deployment serves only to modify semantics of `package` checks and the `__RequirePackage` attribute.

**Note:** Files in `__tests__` directories are automatically excluded from all deployments. See [File package membership](/hack/packages/introduction) for details.

### Force Symbol References (Legacy)

For migration purposes, one can use the `ForceEnableSymbolRefs` option to allow additional "symbolically-referenced" files to be built alongside the active deployment. Note that this only affects [repo authoritative](/hhvm/advanced-usage/repo-authoritative) mode.

## Soft Packages (Migration Support)

Deployments can specify `soft_packages` for files intended to be later excluded:

```toml
[deployments.production]
packages = ["production"]
soft_packages = ["legacy_code"]
```

Accessing soft-deployed package symbols logs a warning instead of throwing an error, allowing identification of dynamic boundary violations.

**Rules:**
- Soft-included packages must be at least soft-deployed
- Hard-included packages must be hard-deployed
- `package_exists()` returns `false` for soft packages. See [Cross Package Usage](/hack/packages/cross-package-calls) for details.
