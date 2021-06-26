<?hh

class Fooz {
  <<__DynamicallyCallable>>
  function baz() {
    register_shutdown_function(onShutdown2<>);
    echo "in Fooz::baz\n";
  }
}

function onShutdownRegisterShutdown_foo() { echo "in foo\n"; }

function onShutdownRegisterShutdown() {
  echo "before register\n";
  register_shutdown_function(onShutdown<>);
  echo "after register\n";
}

function onShutdown() {
  echo "in register\n";
  register_shutdown_function(varray[new Fooz, 'baz']);
}

function onShutdown2() {
  echo "in shutdown 2\n";
}


<<__EntryPoint>>
function main_register_shutdown_function() {
register_shutdown_function(onShutdownRegisterShutdown<>);
register_shutdown_function(onShutdownRegisterShutdown_foo<>);
}
