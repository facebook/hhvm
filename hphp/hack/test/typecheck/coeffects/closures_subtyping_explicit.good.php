<?hh

function f_returns_unsafe_exact_good(): (function()[rx_local]: void) {
  return ()[rx_local] ==> {};
}

function f_returns_unsafe_subtype_good(): (function()[rx_local]: void) {
  return ()[rx] ==> {};
}

function f_returns_safe_exact_good(): (function()[write_props]: void) {
  return ()[write_props] ==> {};
}

function f_returns_safe_subtype_good(): (function()[write_props]: void) {
  return ()[] ==> {};
}

class MyClass {
  public function returns_unsafe_exact_good(): (function()[rx_local]: void) {
    return ()[rx_local] ==> {};
  }

  public function returns_unsafe_subtype_good(): (function()[rx_local]: void) {
    return ()[rx] ==> {};
  }

  public function returns_safe_exact_good(): (function()[write_props]: void) {
    return ()[write_props] ==> {};
  }

  public function returns_safe_subtype_good(): (function()[write_props]: void) {
    return ()[] ==> {};
  }
}
