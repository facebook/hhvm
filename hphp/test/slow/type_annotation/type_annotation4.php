<?hh // strict

// error: non-existent type constant
class C {
}

var_dump(type_structure(C::class, 'T'));
