<?hh

<<__EntryPoint>>
function main() {
  $in = 2;
  var_dump(__hhvm_intrinsics\ReffyNativeMeth::meth(inout $in), $in);
}
