<?hh

class C {
  const type Tnonexistent = int;
  function is_nonexistent(mixed $x): void {
    $result = HH\type_structure_no_throw(static::class, 'Tnonexistent');
    return $result;
  }
}

class D extends C {
  const type Tnonexistent = BlerpityBlerp;
}

<<__EntryPoint>>
function main_is_expression_ta_nonexistent() {
  (new D)->is_nonexistent(1);
  echo "ok\n";
}
