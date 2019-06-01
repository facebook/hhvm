<?hh
<<__EntryPoint>> function main_entry(): void {
  $wrapped = array();
  for ($i = 1; $i < 3; $i++) {
    $wrapped = array($wrapped);
  }
  var_dump($wrapped);
}
