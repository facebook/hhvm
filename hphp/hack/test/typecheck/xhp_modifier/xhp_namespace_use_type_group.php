//// xhp_namespace_use_type_group_def.php
<?hh // strict

namespace foo;

xhp class barone {}
xhp class bartwo {}
//// xhp_namespace_use_type_group_usage.php
<?hh // strict

namespace somethingelse;

use type foo\{barone, bartwo};

function test(): void {
  <barone />;
  <bartwo />;
}
