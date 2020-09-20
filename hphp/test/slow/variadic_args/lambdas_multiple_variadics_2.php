<?hh


<<__EntryPoint>>
function main_lambdas_multiple_variadics_2() {
intProvider((int ...$x, int ...$y) ==> {
  var_dump($x);
  var_dump($y);
});
}
