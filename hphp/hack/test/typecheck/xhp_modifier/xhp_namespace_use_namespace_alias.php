//// xhp_namespace_use_namespace_alias_def.php
<?hh // strict

namespace foo\bar;

xhp class baz extends \XHPTest {}

//// xhp_namespace_use_namespace_alias_usage.php
<?hh // strict

namespace somethingelse;

use namespace foo\bar as b;

function test(): void {
  <b:baz></b:baz>;
}
