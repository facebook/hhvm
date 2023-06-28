<?hh
function h_1499() :mixed{
 var_dump('errored');
}


<<__EntryPoint>>
function main_1499() :mixed{
  set_error_handler(h_1499<>);
  foo_1499(var_dump('123'));
  var_dump('end');
}
