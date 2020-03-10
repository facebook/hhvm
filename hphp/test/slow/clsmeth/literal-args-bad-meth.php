<?hh

class A { static function foo() {} }

<<__EntryPoint>>
function main() {
  $meth = 'foo';
  var_dump(class_meth('A', $meth));
  echo "FAIL\n";
}
