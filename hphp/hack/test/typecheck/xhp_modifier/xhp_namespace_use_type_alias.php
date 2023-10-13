//// xhp_namespace_use_type_alias_def.php
<?hh

namespace foo;

xhp class bar extends \XHPTest {}

class Regular {}

//// xhp_namespace_use_type_alias_usage.php
<?hh

namespace somethingelse;

use type foo\Regular as R;
use type foo\bar as baz;

function test(): void {
  new R();
  <baz />;
}
