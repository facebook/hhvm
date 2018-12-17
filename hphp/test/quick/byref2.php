<?hh

function id(&$x) { return $x; }
function main() {
  $y = 12;
  $w =& id(&$y);
  $w = 7;
  echo $y;
  echo "\n";
}
main();
