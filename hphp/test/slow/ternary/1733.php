<?hh

function add_cssclass($add, $class) {
  $class = (!($class ?? false)) ? $add : $class .= ' ' . $add;
  return $class;
}
<<__EntryPoint>> function main() {
add_cssclass('test', $a);
}
