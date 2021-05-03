<?hh

function f_ret_unsafe_supertype_bad()[oldrx_local]: (function()[oldrx]: void) {
  return ()/*[oldrx_local]*/ ==> {}; // ERROR
}

function f_ret_safe_supertype_bad()[write_props]: (function()[]: void) {
  return ()/*[write_props]*/ ==> {}; // ERROR
}

class MyClass {
  public function ret_unsafe_supertype_bad()[oldrx_local]: (function()[oldrx]: void) {
    return ()/*[oldrx_local]*/ ==> {}; // ERROR
  }

  public function ret_safe_supertype_bad()[write_props]: (function()[]: void) {
    return ()/*[write_props]*/ ==> {}; // ERROR
  }
}

function expects_rx_closure((function()[oldrx]: void) $_): void {}

function passes_rx_local_as_rx_closure()[oldrx_local]: void {
  $f = ()/*[oldrx_local]*/ ==> {};
  expects_rx_closure($f);
}
