<?hh

<<__EntryPoint>>
function main() :mixed{
  require "nested/inner.inc";
  require "nested2/inner.inc";
  require "nested/nested3/inner.inc";

  var_dump(main<>);
  C::inner();
  C2::inner();
  C3::inner();
}
