//// xhp_namespace_use_in_same_namespace_def.php
<?hh

namespace foo;

xhp class bar extends \XHPTest {}

//// xhp_namespace_use_in_same_namespace_usage.php
<?hh

namespace foo;

function test_def(): void {
  <bar />;
}
