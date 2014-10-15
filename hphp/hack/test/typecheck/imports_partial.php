<?hh // partial
echo(require_once dirname(__FILE__) . '/foo.php');
include_once dirname(__FILE__) . '/foo.php';

function foo() {
  $x = require('bar.php');
  $y = $x.(include('baz.php'));
  echo($y);
}
