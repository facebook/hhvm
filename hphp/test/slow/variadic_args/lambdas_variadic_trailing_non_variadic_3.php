<?hh


<<__EntryPoint>>
function main_lambdas_variadic_trailing_non_variadic_3() :mixed{
intProvider((...$_, $y) ==> {
  var_dump($x);
  var_dump($y);
});
}
