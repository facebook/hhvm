//// xhp_namespace_explicit_usage_def.php
<?hh // strict

namespace foo;

xhp class bar {}
//// xhp_namespace_explicit_usage_usage.php
<?hh // strict

namespace baz;

function test_def(): void {
  // qualified name, similar to \foo\bar in non xhp
  <:foo:bar />;
}
