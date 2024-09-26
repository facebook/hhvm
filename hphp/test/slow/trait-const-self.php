<?hh

trait T {
  const type TFoo = self::TVal;
  const type TBar = this::TVal;
}

class P {
  use T;
  const type TVal = bool;
}

class C extends P {
  const type TVal = int;
}

<<__EntryPoint>>
function main() {
  echo 'P::TFoo: '; var_dump(type_structure('P', 'TFoo')['kind']);
  echo 'P::TBar: '; var_dump(type_structure('P', 'TBar')['kind']);
  echo 'P::TFoo: '; var_dump(type_structure('P', __hhvm_intrinsics\launder_value('TFoo'))['kind']);
  echo 'P::TBar: '; var_dump(type_structure('P', __hhvm_intrinsics\launder_value('TBar'))['kind']);

  echo 'C::TFoo: '; var_dump(type_structure('C', 'TFoo')['kind']);
  echo 'C::TBar: '; var_dump(type_structure('C', 'TBar')['kind']);
  echo 'C::TFoo: '; var_dump(type_structure('C', __hhvm_intrinsics\launder_value('TFoo'))['kind']);
  echo 'C::TBar: '; var_dump(type_structure('C', __hhvm_intrinsics\launder_value('TBar'))['kind']);
}
