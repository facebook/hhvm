<?hh

function variadic_hinted_objects(stdClass ...$objects) :mixed{}

function variadic_hinted_scalars(int ...$objects) :mixed{}

function main() :mixed{
  variadic_hinted_scalars(1, 2, 3, 4);
  variadic_hinted_objects(
    new stdClass(), new stdClass(), new stdClass(), new stdClass());
}

<<__EntryPoint>>
function main_typehints() :mixed{
main();
}
