//// defn.php
<?hh

namespace NSTest;
class C {}

//// usage.php
<?hh

use NSTest\C;
function f(): void {
  $a = new C();
}
