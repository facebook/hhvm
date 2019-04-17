<?hh

<<__EntryPoint>>
function main() {
  $x = 1;
  $y = 2;
  var_dump(dict['a' => &$x, 'b' => &$y]);
}
