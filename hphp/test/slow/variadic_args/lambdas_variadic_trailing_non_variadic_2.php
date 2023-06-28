<?hh


<<__EntryPoint>>
function main_lambdas_variadic_trailing_non_variadic_2() :mixed{
intProvider((int ...$x, int $y) ==> {
  var_dump($x);
  var_dump($y);
});
}
