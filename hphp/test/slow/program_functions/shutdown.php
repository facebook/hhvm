<?hh

function shut() :mixed{
  var_dump(__METHOD__);
}

function baz() :mixed{
  var_dump(__METHOD__);
}


<<__EntryPoint>>
function main_shutdown() :mixed{
register_shutdown_function(shut<>);
register_shutdown_function(baz<>);
}
