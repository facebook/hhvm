//// xhp_namespace_use_outside_namespace_4_def.php
<?hh // strict

namespace foo\baz;

xhp class bar extends \XHPTest {}

//// xhp_namespace_use_outside_namespace_4_usage.php
<?hh // strict

namespace foo;

function test_def(): void {
  <baz:bar></baz:bar>;
  <baz:bar />;
}
