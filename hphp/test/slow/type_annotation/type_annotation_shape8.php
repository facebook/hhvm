<?hh

class Orange {
  const type T1 = int;
  const type T2 = ?self::T1;
}

var_dump(type_structure(Orange::class, 'T1'));
var_dump(type_structure(Orange::class, 'T2'));
