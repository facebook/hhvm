# Introduction to Packages

Packages are a file-based mechanism for organizing code into logical units with explicit dependencies and fine-grained deployment control.

## Key Concepts

**Packages** group related files in your codebase (e.g., production code, test code, experimental features).

**Deployments** define which packages to build and run together.

**Package relationships:** Packages explicitly include other packages to access their symbols, creating a dependency graph.

## Quick Example

Packages are configured in a TOML file called `PACKAGES.toml` located at the root of your codebase (next to `.hhconfig`):

```toml PACKAGES.toml
[packages]

[packages.production]
include_paths = ["//flib/"]

[packages.test]
include_paths = ["//flib/test/"]
includes = ["production"]  # test depends on production

[deployments]

[deployments.production]
packages = ["production"]

[deployments.test]
packages = ["test", "production"]  # must include dependencies
```

In this example:
- The `production` package contains all files in `//flib/` (but not those in //flib/test/)
- The `test` package contains files in `//flib/test/` and can access symbols defined in files in the production package
- Two deployments are defined: one for production-only and one that includes tests

## How Packages Work

Each file in your codebase belongs to exactly one package. The same file path cannot appear in multiple packages' `include_paths` clauses.

## File package membership

Files can be assigned to packages in two ways:

### 1. Via `include_paths` in PACKAGES.toml

The `include_paths` clause specifies which files belong to a package:
- Items are relative paths starting with `//` (representing the root directory)
- A path ending with `/` (e.g., `"//flib/"`) includes all files recursively in that directory
- A path without trailing `/` (e.g., `"//flib/foo.php"`) includes only that specific file

### 2. Via `__PackageOverride` file attribute

A file can override its package membership using the `__PackageOverride` attribute:

```hack no-extract
<?hh
<<file: __PackageOverride('production')>>
// This file now belongs to the 'production' package
```

## Precedence rules

When determining which package a file belongs to:
1. If a file has a `__PackageOverride` attribute, it belongs to that package
2. Otherwise, if the full (relative-to-php-root) file path appears in an `include_paths` clause, it belongs to that package
3. Otherwise, the file belongs to the package whose `include_paths` contains the file's nearest ancestor directory (i.e. the longest prefix match)
4. If none of the above apply, the file belongs to the **default package**

### Manual Global Exclusions

It is possible globally exclude files from deployements and/or typechecking. This is controlled for the build by `exclude-pattern` build_config.hdf and for the typechecker by `package_exclude_patterns` in .hhconfig.
- They are excluded from the typechecker (no errors are reported for references to or from these files)
- They are automatically excluded from all deployments, regardless of package membership

By default, that includes files whose path contains the substring `__tests__`.

## Default package

Any file not specified in any package (neither via `include_paths` nor `__PackageOverride`) belongs to the implicit "default" package. Symbols within the default package are not accessible from other packages. The default package:
- Cannot be explicitly defined with the name "default" (the name is reserved)
- Cannot be referenced via its name in `includes` and `soft_includes` lists
- Can be made explicit by creating a package that includes `"//"`

**Example of making default explicit:**
```toml
[packages.orphans]
include_paths = ["//"]
```

This creates a package that captures all files not in other packages.

Once you've defined a set of packages, you can deploy them using [deployments](/hack/packages/deployments).
