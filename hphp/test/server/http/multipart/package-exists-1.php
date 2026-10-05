<?hh

// FILE: PACKAGES.toml
[packages]

[packages.foo]
include_paths = ["//main.php"]
allow_deployed_packages_checking=true

[packages.default]
include_paths = ["//"]

[deployments]

[deployments.my-prod]
packages = ["foo", "default"]


// FILE: main.php

<<__EntryPoint>>
function main(): void {
  // no active deployment is set, so package_exists only checks
  // if the package is defined in the PACKAGES.toml file
  var_dump(package_exists('foo'));
  var_dump(package_exists('bar'));
}
