//// xhp_namespace_use_type_def.php
<?hh

namespace foo;

xhp class bar extends \XHPTest {}

class Regular {}

//// xhp_namespace_use_type_usage.php
<?hh

namespace somethingelse;

use type foo\Regular;
use type foo\bar;

function test(): void {
  new Regular();
  <bar />;
}
