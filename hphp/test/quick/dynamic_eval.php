<?hh

<<__EntryPoint>> function foo(): void {
  $k = 'eval';
  $k('$y = 2'); // dynamic calls to eval are a fatal
  var_dump($y);
}
