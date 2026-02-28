<?hh

<<__EntryPoint>>
function main_yield_inside_async_closure() :mixed{
$fn = async function () {
  yield 123;
};
$fn();
echo "Done\n";
}
