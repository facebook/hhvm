<?hh

function f_ret_unsafe_exact_good()[rx_local]: (function()[rx_local]: void) {
  return ()/*[rx_local*]*/ ==> {};
}

function f_ret_unsafe_subtype_good()[rx]: (function()[rx_local]: void) {
  return ()/*[rx]*/ ==> {};
}

function f_ret_safe_exact_good()[write_props]: (function()[write_props]: void) {
  return ()/*[write_props]*/ ==> {};
}

function f_ret_safe_subtype_good()[]: (function()[write_props]: void) {
  return ()/*[]*/ ==> {};
}

class MyClass {
  public function ret_unsafe_exact_good()[rx_local]: (function()[rx_local]: void) {
    return ()/*[rx_local*]*/ ==> {};
  }

  public function ret_unsafe_subtype_good()[rx]: (function()[rx_local]: void) {
    return ()/*[rx]*/ ==> {};
  }

  public function ret_safe_exact_good()[write_props]: (function()[write_props]: void) {
    return ()/*[write_props]*/ ==> {};
  }

  public function ret_safe_subtype_good()[]: (function()[write_props]: void) {
    return ()/*[]*/ ==> {};
  }
}
