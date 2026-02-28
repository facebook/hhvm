<?hh

function main() :mixed{
  var_dump(abs(-4.2));
  var_dump(abs(5));
  var_dump(abs(-5));
  var_dump(abs("-4.2"));
  var_dump(abs("5"));
  var_dump(abs("-5"));
}


<<__EntryPoint>>
function main_abs() :mixed{
main();
}
