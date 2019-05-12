<?hh
<<__EntryPoint>> function foo(): void {
  $arr = array(12,34,56);
  foreach ($arr as $_ENV => $_FILES) {
    var_dump($_ENV, $_FILES);
  }
}
