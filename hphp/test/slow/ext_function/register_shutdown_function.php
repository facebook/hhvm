<?hh

class Fooz {
  <<__DynamicallyCallable>>
  function baz() :mixed{
    register_shutdown_function(onShutdown2<>);
    echo "in Fooz::baz\n";
  }
}

function onShutdownRegisterShutdown_foo() :mixed{ echo "in foo\n"; }

function onShutdownRegisterShutdown() :mixed{
  echo "before register\n";
  register_shutdown_function(onShutdown<>);
  echo "after register\n";
}

function onShutdown() :mixed{
  echo "in register\n";
  register_shutdown_function(vec[new Fooz, 'baz']);
}

function onShutdown2() :mixed{
  echo "in shutdown 2\n";
}


<<__EntryPoint>>
function main_register_shutdown_function() :mixed{
register_shutdown_function(onShutdownRegisterShutdown<>);
register_shutdown_function(onShutdownRegisterShutdown_foo<>);
}
