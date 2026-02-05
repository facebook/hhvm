<?hh

<<__EntryPoint>> function foo(): void {
  $vals = vec[1,5,2,1.54,-123.3,1256.6,NAN,-NAN,INF,-INF,(float)(((1<<52)-1)<<11),(float)PHP_INT_MIN,null /*throws*/];
  foreach ($vals as $v) {
    var_dump(~($v is null ? $v : (int)$v));
  }
}
