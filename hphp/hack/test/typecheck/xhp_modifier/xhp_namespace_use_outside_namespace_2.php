//// xhp_namespace_use_outside_namespace_2_def.php
<?hh

namespace foo\baz;

xhp class bar extends \XHPTest {}

//// xhp_namespace_use_outside_namespace_2_usage.php
<?hh

function test_def(): void {
  <foo:baz:bar></foo:baz:bar>;
  <foo:baz:bar />;
}
