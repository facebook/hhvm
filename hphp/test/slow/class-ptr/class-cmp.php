<?hh

class foo {}
class bar {}

function baz($a, $b, $c, $d, $e) :mixed{
  var_dump($a === $b); var_dump($a == $b);
  var_dump($a < $b);   var_dump($a <= $b);
  var_dump($a > $b);   var_dump($a >= $b);

  var_dump($a === $c); var_dump($a == $c);
  var_dump($a < $c);   var_dump($a <= $c);
  var_dump($a > $c);   var_dump($a >= $c);

  var_dump($a === $d); var_dump($a == $d);
  var_dump($a < $d);   var_dump($a <= $d);
  var_dump($a > $d);   var_dump($a >= $d);

  var_dump($a === $e); var_dump($a == $e);
  var_dump($a < $e);   var_dump($a <= $e);
  var_dump($a > $e);   var_dump($a >= $e);

  var_dump(
    ($e === 'bar' ? __hhvm_intrinsics\create_class_pointer('foo') : __hhvm_intrinsics\create_class_pointer('bar')) === ($d === 'foo' ? 'foo' : 'bar')
  );
  var_dump(
    ($e === 'bar' ? __hhvm_intrinsics\create_class_pointer('foo') : __hhvm_intrinsics\create_class_pointer('bar')) !== ($d === 'foo' ? 'foo' : 'bar')
  );
}

<<__EntryPoint>>
function main() :mixed{
  baz(__hhvm_intrinsics\create_class_pointer('foo'), __hhvm_intrinsics\create_class_pointer('foo'), __hhvm_intrinsics\create_class_pointer('bar'), 'foo', 'bar');
}
