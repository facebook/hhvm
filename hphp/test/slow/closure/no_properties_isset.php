<?hh


<<__EntryPoint>>
function main_no_properties() {
  $func = function() {};
  isset($func->a);
}
