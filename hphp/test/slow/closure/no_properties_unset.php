<?hh


<<__EntryPoint>>
function main_no_properties() {
  $func = function() {};
  unset($func->a);
}
