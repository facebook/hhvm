<?hh

function shut() {
  var_dump(__METHOD__);
}

function baz() {
  var_dump(__METHOD__);
}


<<__EntryPoint>>
function main_shutdown() {
register_shutdown_function(fun("shut"));
register_shutdown_function(fun("baz"));
}
