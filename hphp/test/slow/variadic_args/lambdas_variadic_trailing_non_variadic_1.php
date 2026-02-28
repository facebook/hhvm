<?hh


<<__EntryPoint>>
function main_lambdas_variadic_trailing_non_variadic_1() :mixed{
intProvider((...$x, $y) ==> {
  var_dump($x);
  var_dump($y);
});
}
