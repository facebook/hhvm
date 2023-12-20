Packages are an extension to modules for HHVM and Hack which allows developers to more easily configure which files to build and deploy.

## Package definitions
A **package** is defined by a set of modules meant to be deployed together. Packages are configured in a toml file called `PACKAGES.toml` located at the root of your codebase (next to .hhconfig).

```toml PACKAGES.toml
[packages]

[packages.production]
uses=["prod.*", "my_prod"] # Package contains all modules that start with `prod`, and the module "my_prod".

[packages.test]
uses=["test.*"]
includes=["production"] # Package depends on the production package
```

Every module can be in at most one package, so the same module cannot be part of two packages. Therefore, the same glob cannot appear in the uses clause of two different modules.

## Overlaps in module globs
- It is an error for the same module glob to appear in multiple package definitions.  Given a module with name foo.bar, we resolve what package the module belongs to with the following precedence:
- If a package lists foo.bar directly in its use clause, foo.bar belongs to that package. If multiple packages list foo.bar directly, we throw an error at package definition.
- If no package directly references the module, but a package has a prefix which matches foo.bar (in this case, foo.* for example), foo.bar belongs to the package with that prefix. If multiple packages list a prefix of the module, we take the most specific prefix, i.e. the longest prefix that matches.
- If no package references the module in its use clause, the module belongs to the default package.

Once you've defined a set of packages, you can deploy them using [deployments](./deployments.md).
