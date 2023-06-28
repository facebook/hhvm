<?hh

function foo() :mixed{
  var_dump(is_array($_GET));
  var_dump($_GET);
}


<<__EntryPoint>>
function main_is_typeop_with_pseudoglobal() :mixed{
foo();
}
