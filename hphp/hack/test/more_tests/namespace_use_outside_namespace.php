//// defn.php
<?hh // strict

namespace NSTest;
class C {}

//// usage.php
<?hh // strict

use NSTest\C;
function f(): void {
  $a = new C();
}
