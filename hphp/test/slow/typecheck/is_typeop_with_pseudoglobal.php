<?hh

function foo() :mixed{
  var_dump(is_array(\HH\global_get('_GET')));
  var_dump(\HH\global_get('_GET'));
}


<<__EntryPoint>>
function main_is_typeop_with_pseudoglobal() :mixed{
foo();
}
