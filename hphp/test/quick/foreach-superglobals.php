<?hh
<<__EntryPoint>> function foo(): void {
  $arr = vec[12,34,56];
  foreach ($arr as $_ENV => $_FILES) {
    var_dump(\HH\global_get('_ENV'), \HH\global_get('_FILES'));
  }
}
