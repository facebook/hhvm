<?hh

function exn_throw($exn) :mixed{
  throw new Exception('throwing second');
}
<<__EntryPoint>>
function main() :mixed{
  set_exception_handler(exn_throw<>);
  throw new Exception('throwing first');
}
