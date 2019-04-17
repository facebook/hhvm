<?hh

<<__EntryPoint>>
function main() {
  $x = 1;
  $y = 2;
  var_dump(['a' => &$x, 'b' => &$y]);
}
