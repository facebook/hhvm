//// xhp_namespace_use_namespace_alias_def.php
<?hh

namespace foo\bar;

xhp class baz extends \XHPTest {}

//// xhp_namespace_use_namespace_alias_usage.php
<?hh

namespace somethingelse;

use namespace foo\bar as b;

function test(): void {
  <b:baz></b:baz>;
}
