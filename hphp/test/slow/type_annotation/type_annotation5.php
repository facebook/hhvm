<?hh // strict

// error: abstract type const
abstract class C {
  abstract const type T;
}

var_dump(type_structure(C::class, 'T'));
