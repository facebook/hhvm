<?hh

namespace ShutdownTest;

<<__DynamicallyCallable>>
function test() {
  \var_dump('function');
}

class Test {
  <<__DynamicallyCallable>>
  function handleInstance() {
    \var_dump('Method - instance');
  }

  <<__DynamicallyCallable>>
  static function handleStatic() {
    \var_dump('Method - static');
  }
}

<<__EntryPoint>>
function main_register_shutdown_function_simple() {
  \register_shutdown_function(__NAMESPACE__ . '\test');
  \register_shutdown_function(varray[new Test, 'handleInstance']);
  \register_shutdown_function(varray[__NAMESPACE__ . '\Test', 'handleStatic']);
  \register_shutdown_function(function () {
    \var_dump('Lambda');
  });
  \register_shutdown_function(function (...$args) {
    \var_dump($args);
  });
}
