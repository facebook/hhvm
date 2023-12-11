<?hh

namespace ShutdownTest;

<<__DynamicallyCallable>>
function test() :mixed{
  \var_dump('function');
}

class Test {
  <<__DynamicallyCallable>>
  function handleInstance() :mixed{
    \var_dump('Method - instance');
  }

  <<__DynamicallyCallable>>
  static function handleStatic() :mixed{
    \var_dump('Method - static');
  }
}

<<__EntryPoint>>
function main_register_postsend_function_simple() :mixed{
  \register_postsend_function(__NAMESPACE__ . '\test');
  \register_postsend_function(vec[new Test, 'handleInstance']);
  \register_postsend_function(vec[__NAMESPACE__ . '\Test', 'handleStatic']);
  \register_postsend_function(function () {
    \var_dump('Lambda');
  });
  \register_postsend_function(function (...$args) {
    \var_dump($args);
  });
}
