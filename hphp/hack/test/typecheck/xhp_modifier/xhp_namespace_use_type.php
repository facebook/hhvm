//// xhp_namespace_use_type_def.php
<?hh // strict

namespace foo;

xhp class bar {}

class Regular {}
//// xhp_namespace_use_type_usage.php
<?hh // strict

namespace somethingelse;

use type foo\Regular;
use type foo\bar;

function test(): void {
  new Regular();
  <bar />;
}
