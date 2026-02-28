# Configuration Reference

This page provides a comprehensive reference for all fields in `PACKAGES.toml`.

## File Structure

`PACKAGES.toml` contains two main sections: `[packages]` and `[deployments]`.

```toml
# PACKAGES.toml
[packages]

[packages.production]
include_paths = ["//flib/prod/"]
soft_includes = ["test_actually_prod"]

[packages.test]
include_paths = ["//flib/test/"]
includes = ["production"]

[packages.test_actually_prod]
includes = ["production"]

[deployments]

[deployments.production]
packages = ["production"]
soft_packages = ["test_actually_prod"]

[deployments.test]
packages = ["test", "test_actually_prod", "production"]
```

## Packages Section

Defines all packages and their relationships. Each package is a nested table with a unique name.

### Package Name

Package names must be unique. The reserved name `"default"` cannot be used.

```toml
[packages.my_package]  # "my_package" is the package name
```

### `include_paths`

**Type:** List of strings (optional)

**Purpose:** Specifies files and directories belonging to this package.

**Syntax:**
- Paths start with `//`, the root directory where `PACKAGES.toml` is located
- A path ending with `/` (e.g., `"//flib/"`) includes all files recursively in that directory
- A path without trailing `/` (e.g., `"//flib/foo.php"`) includes only that specific file
- Paths must be normalized (no `"../"` or `"./"`)

**Rules:**
- Each path must be unique across all packages (parse error otherwise)
- Each path must resolve to a valid location (parse error otherwise)

**Example:**
```toml
[packages.production]
include_paths = [
  "//flib/",                    # All files in flib/ recursively
  "//config/prod_config.php"    # Single specific file
]
```

### `includes`

**Type:** List of strings (optional)

**Purpose:** Lists packages this package depends on. Code can access included package symbols.

**Syntax:** Package names, double-quoted and comma-separated

**Rules:** Must be **explicitly closed with respect to transitivity** (if A includes B, and B includes C, then A must explicitly list C)

**Example:**
```toml
[packages.core]
include_paths = ["//core/"]

[packages.utils]
include_paths = ["//utils/"]
includes = ["core"]

[packages.app]
include_paths = ["//app/"]
includes = ["utils", "core"]  # Must explicitly include both utils AND core
```

### `soft_includes`

**Type:** List of strings (optional)

**Purpose:** Lists dynamic dependencies for migration. Accessing soft-included package symbols raises typechecker errors.

**Use Case:** When removing dependencies between packages. Change `includes = ["B"]` to `soft_includes = ["B"]` to identify all statically known places where A accesses B's symbols.

**Syntax:** Package names, double-quoted and comma-separated

**Rules:** Soft-included packages must be at least soft-deployed where the including package is present

**Example:**
```toml
[packages.production]
include_paths = ["//flib/prod/"]
soft_includes = ["migrate_me"]  # Want to remove this dependency

[packages.migrate_me]
include_paths = ["//flib/legacy/"]
includes = ["production"]
```

Here:
- Typechecker errors on production code accessing `migrate_me` symbols
- At runtime, `migrate_me` is still deployed (via `soft_packages`), so existing dynamic references won't fatal
- Developers systematically fix typechecker errors to remove the dependency

## Deployments Section

Defines which packages are built and deployed together. Each deployment is a nested table with a unique name.

### Deployment Name

Deployment names must be unique.

```toml
[deployments.production]  # "production" is the deployment name
```

### `packages`

**Type:** List of strings (required)

**Purpose:** Lists packages in this deployment.

**Syntax:** Package names, double-quoted and comma-separated

**Rules:** Must be **transitively closed** (if A is deployed and A includes B, then B must be deployed)

**Example:**
```toml
[packages.core]
include_paths = ["//core/"]

[packages.utils]
include_paths = ["//utils/"]
includes = ["core"]

[packages.app]
include_paths = ["//app/"]
includes = ["utils", "core"]

[deployments.app]
# All three must be listed because of transitive dependencies
packages = ["app", "utils", "core"]
```

### `soft_packages`

**Type:** List of strings (optional)

**Purpose:** Packages deployed at runtime that log warnings when accessed.

**Use Case:** When migrating away from a dependency, move a package from `packages` to `soft_packages`. It remains deployed (preventing runtime fatals) while ones identifies and fix all accesses.

**Syntax:** Package names, double-quoted and comma-separated

**Rules:**
- Soft-included packages must be in either the `soft_packages` or `packages` list
- `package soft_package_name` returns `false` for soft packages

**Example:**
```toml
[packages.production]
include_paths = ["//flib/prod/"]
soft_includes = ["legacy"]

[packages.legacy]
include_paths = ["//flib/legacy/"]

[deployments.production]
packages = ["production"]
soft_packages = ["legacy"]  # legacy is deployed but accessing it logs warnings
```

## Complete Example

Here's a complete example showing all configuration options:

```toml
# PACKAGES.toml

[packages]

# Core production package
[packages.core]
include_paths = ["//flib/core/"]

# Production utilities that depend on core
[packages.prod_utils]
include_paths = ["//flib/utils/"]
includes = ["core"]

# Main production application
[packages.production]
include_paths = ["//flib/prod/"]
includes = ["prod_utils", "core"]
soft_includes = ["legacy_feature"]  # Migrating away from this

# Legacy feature being phased out
[packages.legacy_feature]
include_paths = ["//flib/legacy/"]
includes = ["core"]

# Test package
[packages.test]
include_paths = ["//flib/test/"]
includes = ["production", "prod_utils", "core"]

# Files in test that temporarily need to be in production
[packages.test_actually_prod]
# No include_paths - files use __PackageOverride to join this package
includes = ["production", "prod_utils", "core"]

[deployments]

# Production deployment (minimal)
[deployments.production]
packages = ["production", "prod_utils", "core"]
soft_packages = ["legacy_feature", "test_actually_prod"]

# Test deployment (includes everything)
[deployments.test]
packages = [
  "test",
  "production",
  "prod_utils",
  "core",
  "test_actually_prod",
  "legacy_feature"
]
```
