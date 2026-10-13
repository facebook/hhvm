<?hh

case type StrOrInt = string | int;

class C {}

function own_lazy(): StrOrInt {
  return __hhvm_intrinsics\launder_value(C::class);
}

function own_class(): StrOrInt {
  return __hhvm_intrinsics\launder_value(HH\classname_to_class(C::class));
}

function own_class_known(): StrOrInt {
  return HH\classname_to_class(C::class);
}

class Base {
  function lazy(): StrOrInt { return 1; }
  function cls(): StrOrInt { return 1; }
  function clsKnown(): StrOrInt { return 1; }
}

class Derived extends Base {
  function lazy(): mixed {
    return __hhvm_intrinsics\launder_value(C::class);
  }

  function cls(): mixed {
    return __hhvm_intrinsics\launder_value(HH\classname_to_class(C::class));
  }

  function clsKnown(): mixed {
    return HH\classname_to_class(C::class);
  }
}

<<__EntryPoint>>
function main(): void {
  __hhvm_intrinsics\debug_var_dump_lazy_class(own_lazy());
  __hhvm_intrinsics\debug_var_dump_lazy_class(own_class());
  __hhvm_intrinsics\debug_var_dump_lazy_class(own_class_known());

  $d = new Derived();
  __hhvm_intrinsics\debug_var_dump_lazy_class($d->lazy());
  __hhvm_intrinsics\debug_var_dump_lazy_class($d->cls());
  __hhvm_intrinsics\debug_var_dump_lazy_class($d->clsKnown());
}
