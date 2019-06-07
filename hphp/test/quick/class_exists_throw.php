<?hh

function __autoload() {
  throw new Exception('sup');
  include 'class_exists_throw1.inc';
}

function main() {
  echo class_exists('nufin');
  echo "\n";
}
try { main(); } catch (Exception $x) { echo $x->getMessage(); echo "\n"; }
echo "done\n";
