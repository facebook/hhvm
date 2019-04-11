<?hh

<<__EntryPoint>>
function main() {
  $a = null;
  $aa = array(&$a);
  $a = array(&$aa);
  var_dump(count($a, COUNT_RECURSIVE));
}
