<?hh

class NoCtor implements \HH\FunctionAttribute {
  public int $x = 42;
}

function call_implicitly_pure_ctor()[]: NoCtor {
  return new NoCtor(); // ok
}

class WithPureCtor implements \HH\FunctionAttribute {
  public function __construct()[] {}
}

function call_explicitly_pure_ctor()[]: WithPureCtor {
  return new WithPureCtor(); // ok
}

class WithImpureCtor implements \HH\FunctionAttribute {
  public function __construct()[zoned] {}
}

function call_impure_ctor_bad()[]: WithImpureCtor {
  return new WithImpureCtor(); // error
}

function call_impure_ctor_good()[zoned]: WithImpureCtor {
  return new WithImpureCtor(); // ok
}

<<NoCtor>>
function attr_no_ctor(): void {}

<<WithPureCtor>>
function attr_pure_ctor(): void {}

<<WithImpureCtor>>
function attr_impure_ctor(): void {}
