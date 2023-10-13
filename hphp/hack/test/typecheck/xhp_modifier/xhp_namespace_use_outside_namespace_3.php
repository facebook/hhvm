//// xhp_namespace_use_outside_namespace_3_def.php
<?hh

namespace foo {
  xhp class bar extends \XHPTest {}
}

//// xhp_namespace_use_outside_namespace_3_usage.php
<?hh

function test_def(): void {
  <foo:bar />;
  <foo:bar></foo:bar>;
}
