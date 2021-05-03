<?hh

function f_ret_unsafe_exact_good()[oldrx_local]: (function()[oldrx_local]: void) {
  return ()/*[oldrx_local*]*/ ==> {};
}

function f_ret_unsafe_subtype_good()[oldrx]: (function()[oldrx_local]: void) {
  return ()/*[oldrx]*/ ==> {};
}

function f_ret_safe_exact_good()[write_props]: (function()[write_props]: void) {
  return ()/*[write_props]*/ ==> {};
}

function f_ret_safe_subtype_good()[]: (function()[write_props]: void) {
  return ()/*[]*/ ==> {};
}

class MyClass {
  public function ret_unsafe_exact_good()[oldrx_local]: (function()[oldrx_local]: void) {
    return ()/*[oldrx_local*]*/ ==> {};
  }

  public function ret_unsafe_subtype_good()[oldrx]: (function()[oldrx_local]: void) {
    return ()/*[oldrx]*/ ==> {};
  }

  public function ret_safe_exact_good()[write_props]: (function()[write_props]: void) {
    return ()/*[write_props]*/ ==> {};
  }

  public function ret_safe_subtype_good()[]: (function()[write_props]: void) {
    return ()/*[]*/ ==> {};
  }
}
