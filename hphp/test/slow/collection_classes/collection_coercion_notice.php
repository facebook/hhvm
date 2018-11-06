<?hh

<<__EntryPoint>>
function main() {
  __hhvm_intrinsics\dummy_array_builtin(Map {'foo' => 'bar', 'baz' => null});
  __hhvm_intrinsics\dummy_array_builtin(Vector {1, 2, 'foo'});
  __hhvm_intrinsics\dummy_array_builtin(Set {1, 2, 'foo'});
  __hhvm_intrinsics\dummy_array_builtin(ImmMap {'foo' => 'bar', 'baz' => null});
  __hhvm_intrinsics\dummy_array_builtin(ImmVector {1, 2, 'foo'});
  __hhvm_intrinsics\dummy_array_builtin(ImmSet {1, 2, 'foo'});
  __hhvm_intrinsics\dummy_array_builtin(Pair {1, 2});
}
