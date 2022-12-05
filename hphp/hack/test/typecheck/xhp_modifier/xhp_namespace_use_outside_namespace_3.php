//// xhp_namespace_use_outside_namespace_3_def.php
<?hh // strict

namespace foo {
  xhp class bar extends \XHPTest {}
}

//// xhp_namespace_use_outside_namespace_3_usage.php
<?hh // strict

function test_def(): void {
  <foo:bar />;
  <foo:bar></foo:bar>;
}
