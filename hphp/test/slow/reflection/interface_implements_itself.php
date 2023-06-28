<?hh

namespace {
  interface Iface {}
}

namespace N {
  interface Iface {}
}

namespace T {
  interface Iface {}
}

namespace {
function main() :mixed{
  $i  = new ReflectionClass('Iface');
  $ni = new ReflectionClass('N\Iface');
  $ti = new ReflectionClass('\T\Iface');

  \var_dump($i->implementsInterface('Iface'));
  \var_dump($ni->implementsInterface('\N\Iface'));
  \var_dump($ti->implementsInterface('T\Iface'));

  \var_dump($i->implementsInterface('N\Iface'));
  \var_dump($ti->implementsInterface('N\Iface'));
  \var_dump($ni->implementsInterface('Iface'));
}
<<__EntryPoint>> function main_entry(): void {
main();
}
}
