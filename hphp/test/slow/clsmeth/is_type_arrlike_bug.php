<?hh

<<__EntryPoint>>
function main(): void {
  var_dump(HH\is_any_array(vec[1, 2, 3]));
  var_dump(HH\is_any_array(class_meth(Vector::class, 'fromArray')));
}
