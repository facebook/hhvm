<?hh


<<__EntryPoint>>
function main_lambdas_multiple_variadics_1() {
intProvider((...$x, ...$y) ==> {
  var_dump($x);
  var_dump($y);
});
}
