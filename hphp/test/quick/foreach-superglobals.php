<?hh
<<__EntryPoint>> function foo(): void {
  $arr = vec[12,34,56];
  foreach ($arr as $_ENV => $_FILES) {
    var_dump($_ENV, $_FILES);
  }
}
