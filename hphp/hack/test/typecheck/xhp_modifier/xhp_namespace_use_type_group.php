//// xhp_namespace_use_type_group_def.php
<?hh

namespace foo;

xhp class barone extends \XHPTest {}
xhp class bartwo extends \XHPTest {}

//// xhp_namespace_use_type_group_usage.php
<?hh

namespace somethingelse;

use type foo\{barone, bartwo};

function test(): void {
  <barone />;
  <bartwo />;
}
