<?hh

function f(mixed $x): void {
  var_dump($x is stdClass);
}

function g(mixed $x): void {
  var_dump($x is ?stdClass);
}


<<__EntryPoint>>
function main_is_expression_stdClass() :mixed{
f(1);
f(new stdClass());
f(null);

g(1);
g(new stdClass());
g(null);
}
