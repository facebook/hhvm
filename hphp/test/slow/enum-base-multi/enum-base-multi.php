<?hh

function test<reify T>(T $a) { return $a; }
function test2(Foo $a) { return $a; }

<<__EntryPoint>>
function main() {
  set_error_handler(() ==> { throw new Exception(); });
  var_dump(test<Foo>('a'));
  var_dump(test<Foo>(__hhvm_intrinsics\launder_value('a')));

  var_dump(test2('a'));
  var_dump(test2(__hhvm_intrinsics\launder_value('a')));

  // N.B. This behavior is incorrect-- these calls should throw but the way that
  //      we handle reified checks for enums with classname<T> bases is
  //      currently to treat them as having type arraykey.
  try { var_dump(test<Foo>(12)); } catch (Exception $e) { echo "ok\n"; }
  try { var_dump(test<Foo>(__hhvm_intrinsics\launder_value(12))); } catch (Exception $e) { echo "ok\n"; }

  try { var_dump(test2(12)); } catch (Exception $e) { echo "ok\n"; }
  try { var_dump(test2(__hhvm_intrinsics\launder_value(12))); } catch (Exception $e) { echo "ok\n"; }
}
