<?hh

function Test() :mixed{
  var_dump(__FUNCTION__);
}

<<__EntryPoint>>
function main(): void {
  Test();
  var_dump(__FUNCTION__);
}
