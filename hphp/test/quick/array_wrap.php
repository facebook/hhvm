<?hh
<<__EntryPoint>> function main_entry(): void {
  $wrapped = varray[];
  for ($i = 1; $i < 3; $i++) {
    $wrapped = varray[$wrapped];
  }
  var_dump($wrapped);
}
