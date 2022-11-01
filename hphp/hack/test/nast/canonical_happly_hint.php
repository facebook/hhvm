<?hh

function canon_int(int $x): void  {}

function canon_bool(bool $x): void  {}

function canon_float(float $x): void  {}

function canon_string(string $x): void  {}

function canon_darray(darray<arraykey,int> $x): void  {}

function canon_varray(varray<arraykey> $x): void  {}

function canon_varray_or_darray1(varray_or_darray<arraykey> $x): void  {}

function canon_varray_or_darray2(varray_or_darray<arraykey,int> $x): void  {}

function canon_vec_or_dict(vec_or_dict<arraykey> $x): void  {}

function canon_vec_or_dict1(vec_or_dict<arraykey,int> $x): void  {}

function canon_null(null $x): void  {}

function canon_num(num $x): void  {}

function canon_resource(resource $x): void  {}

function canon_arraykey(arraykey $x): void  {}

function canon_mixed(mixed $x): void  {}

function canon_nonnull(nonnull $x): void  {}

function canon_dynamic(dynamic $x): void  {}

function canon_nothing(nothing $x): void  {}

function canon_classname(classname $x): void {}

class C {
  public function canon_this(this $x): void {}
}

class D<T> {
  public function canon_class_typaram(T $x): void {}
}

function canon_function_typaram<T>(T $x): void {}
