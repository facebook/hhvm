<?hh
namespace test;

function a(\Throwable $t) {
  \var_dump(\get_class($t));
}


<<__EntryPoint>>
function main_throwable_namespace_002() {
a(new \RuntimeException('foo'));
}
