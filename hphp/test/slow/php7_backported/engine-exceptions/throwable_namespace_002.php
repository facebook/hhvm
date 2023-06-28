<?hh
namespace test;

function a(\Throwable $t) :mixed{
  \var_dump(\get_class($t));
}


<<__EntryPoint>>
function main_throwable_namespace_002() :mixed{
a(new \RuntimeException('foo'));
}
