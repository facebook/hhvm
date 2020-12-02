<?hh

class A { static function foo() {} }

<<__EntryPoint>>
function main() {
  $cls = 'A';
  var_dump(class_meth($cls, 'foo'));
  echo "FAIL\n";
}
