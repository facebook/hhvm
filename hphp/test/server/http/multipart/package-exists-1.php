<?hh

// FILE: PACKAGES.toml
[packages]

[packages.foo]
uses = ["a.*"]

[packages.bar]
uses = ["b.*"]
includes = ["foo"]

[packages.default]
uses = ["*"]

[deployments]

[deployments.my-prod]
packages = ["foo", "default"]
domains = ['.*\.facebook\.com$', '.*\.fbinfra\.net$']


// FILE: module_page.php

new module a {}

// FILE: main.php

module a;

<<__EntryPoint>>
function main(): void {
  var_dump(package_exists('foo'));
  var_dump(package_exists('bar'));
}
