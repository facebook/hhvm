<?hh
<<__EntryPoint>> function main_entry(): void {
  $wrapped = vec[];
  for ($i = 1; $i < 3; $i++) {
    $wrapped = vec[$wrapped];
  }
  var_dump($wrapped);
}
