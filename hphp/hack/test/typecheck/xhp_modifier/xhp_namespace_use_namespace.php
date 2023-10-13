//// xhp_namespace_use_namespace_def.php
<?hh

namespace foo\bar;

xhp class baz extends \XHPTest {}

//// xhp_namespace_use_namespace_usage.php
<?hh

namespace somethingelse;

use namespace foo\bar;

function test(): void {
  <bar:baz></bar:baz>;
}
