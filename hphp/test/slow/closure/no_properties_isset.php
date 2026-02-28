<?hh


<<__EntryPoint>>
function main_no_properties() :mixed{
  $func = function() {};
  isset($func->a);
}
