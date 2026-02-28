<?hh

<<__EntryPoint>>
function main() :mixed{
  require "nested2/inner.inc";

  var_dump(main<>);
  C2::inner();
}
