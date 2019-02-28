<?hh // partial
function takes_int(int $x) : void {}
function foo($x) {
  hh_show($x);
  takes_int($x);
  hh_show($x);
}
