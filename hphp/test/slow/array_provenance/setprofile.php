<?hh


<<__NEVER_INLINE>>
function foo() {
  return __hhvm_intrinsics\launder_value(42);
}


<<__EntryPoint>>
function main() {

  fb_setprofile(
    function ($why, $func, $info) {
      if ($func === "foo") {
        var_dump(HH\get_provenance($info));
      }
    }
  );

  foo();
}
