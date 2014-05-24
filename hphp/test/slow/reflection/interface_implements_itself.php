<?php

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
function main() {
  $i  = new ReflectionClass('IFace');
  $ni = new ReflectionClass('N\IFace');
  $ti = new ReflectionClass('\T\IFace');

  var_dump($i->implementsInterface('IfAcE'));
  var_dump($ni->implementsInterface('\N\IfAcE'));
  var_dump($ti->implementsInterface('T\IfAcE'));

  var_dump($i->implementsInterface('N\Iface'));
  var_dump($ti->implementsInterface('N\Iface'));
  var_dump($ni->implementsInterface('Iface'));
}
main();
}
