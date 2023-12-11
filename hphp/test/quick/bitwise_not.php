<?hh

<<__EntryPoint>> function foo(): void {
  $vals = vec[1,5,2,1.54,-123.3,1256.6,null /*throws*/];
  foreach ($vals as $v) {
    var_dump(~($v is null ? $v : (int)$v));
  }
}
