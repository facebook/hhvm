//// xhp_namespace_explicit_usage_def.php
<?hh

namespace foo;

xhp class bar extends \XHPTest {}

//// xhp_namespace_explicit_usage_usage.php
<?hh

namespace baz;

function test_def(): void {
  // qualified name, similar to \foo\bar in non xhp
  <:foo:bar />;
}
