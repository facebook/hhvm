This page has detailed documentation of each field in PACKAGES.toml.

```
# PACKAGES.toml
[packages]

[packages.prod]
uses = ["prod.*"]
includes = ["default"]
soft_includes = ["test_soft"]

[packages.test_strict]
uses = ["test.*"]
includes = ["prod", "test_soft", "default"]

# Code that’s in test but may be dynamically called from prod
[packages.test_soft]
uses = ["test_soft.*"]
includes = ["prod", "default"]

[packages.default] # default package with everything else
uses=[".*"]

[deployments.test-website]
packages = ["test_strict", "test_soft", "prod", "default"]
domains = ['.*test\.my-website\.com']

[deployments.prod-website]
packages = ["prod", "default"]
soft_packages = ["test_soft"]
domains = ['.*\.my-website\.com']
```


A PACKAGES.toml file contains two main sections: packages and deployments.
The packages section lists all available packages and their respective dependencies. Each package is defined as a nested table with a unique name, and can have two fields:

**uses:** a list of module globs that this package depends on. For example, `prod.*` means that this package uses the module `prod` and all modules that start with `prod.*`. A module can only be part of a single package, and match the most specific glob when deciding which package it’s a part of.

**includes:** a string list of other package names that this package depends on. Any package `A` that is included by a package `B` must be deployed in any deployment that B is present in.

**soft_includes:** a string list of package names that this package may depend on dynamically. Accessing a package that’s soft included from this package will raise a Hack error. Any package `A` that is soft included by a package `B` must be at least soft deployed in any deployment that `B` is present in.

The deployments section lists all deployments and their respective packages and domains. Each deployment is defined as a nested table with a unique name, and can have two fields:
packages: a list of packages that are in this deployment. The list must be transitively closed, that is, any package that is (non softly) included by any package in this list must also be in the list.

**soft_packages:** a list of packages that are included in this deployment at runtime, but will log when accessed. Any packages that are soft included by any package in the deployment must be either in this list or the list of hard included packages above.
domains: a list of domains where this deployment is accessible on sandboxes. A domain is listed as a list of regular expressions. In sandboxes, HHVM matches these regular expressions on the Host field of any request. When choosing which deployment is considered active in sandbox mode, HHVM will take the first eligible deployment which contains a domain regex that matches a given host.

Note that any code that isn’t in a module is considered part of the default module, which is simply a module with the name `default`. The default module can be globbed into any package, the same way any other module can be:

```
[[packages.with_default]]
uses=["default", "prod.*"] # prod can call code outside of modules

[[packages.no_default]] # foo cannot call code outside of modules
uses=["foo"]
```


A common use case is to put the default module into a default package which globs ".*", which contains the default module. Then you can include that package in all deployments (and have other packages depend on it).

```
[packages.default] # default package with everything else
uses=[".*"]


[packages.foo]
uses=["foo.*"]
includes=["default"]
```
