<?hh

<<file: __EnableUnstableFeatures('union_intersection_type_hints', 'strict_switch')>>

interface Intf {}

class GenericClass<T> implements Intf {
  public function __construct(public T $data)[] {}
}

class DerivedGenClass<T> extends GenericClass<T> {
  public function __construct(public T $data)[] {
    parent::__construct($data);
  }
}

class IntClass implements Intf {
  public function __construct(public int $data)[] {}
}

enum class DerivedEnumClass: Intf {
  GenericClass<int> GenInt = new GenericClass(42);
  GenericClass<string> GenString = new GenericClass('red');
  DerivedGenClass<int> DerGenInt2 = new DerivedGenClass(2021);
  IntClass LabelIntClass = new IntClass(0);
}

<<__StrictSwitch>>
function generic_enum_switch<T>(\HH\MemberOf<DerivedEnumClass, GenericClass<T>> $x): void {
  switch ($x) {
    case DerivedEnumClass::GenInt:
    case DerivedEnumClass::DerGenInt2:
    case DerivedEnumClass::GenString:
        return;
  }
}

<<__StrictSwitch>>
function instantiated_enum_switch(\HH\MemberOf<DerivedEnumClass, GenericClass<int>> $x): void {
  switch ($x) {
    case DerivedEnumClass::GenInt:
    case DerivedEnumClass::DerGenInt2:
      return;
  }
}
