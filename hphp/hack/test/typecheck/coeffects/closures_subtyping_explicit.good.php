<?hh

function f_returns_unsafe_exact_good(): (function()[oldrx_local]: void) {
  return ()[oldrx_local] ==> {};
}

function f_returns_unsafe_subtype_good(): (function()[oldrx_local]: void) {
  return ()[oldrx] ==> {};
}

function f_returns_safe_exact_good(): (function()[write_props]: void) {
  return ()[write_props] ==> {};
}

function f_returns_safe_subtype_good(): (function()[write_props]: void) {
  return ()[] ==> {};
}

class MyClass {
  public function returns_unsafe_exact_good(): (function()[oldrx_local]: void) {
    return ()[oldrx_local] ==> {};
  }

  public function returns_unsafe_subtype_good(): (function()[oldrx_local]: void) {
    return ()[oldrx] ==> {};
  }

  public function returns_safe_exact_good(): (function()[write_props]: void) {
    return ()[write_props] ==> {};
  }

  public function returns_safe_subtype_good(): (function()[write_props]: void) {
    return ()[] ==> {};
  }
}
