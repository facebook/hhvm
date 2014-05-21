<?php

namespace {
  interface I {}
}

namespace N {
  interface I {}
}

namespace T {
  interface I {}
}

namespace {
  $i = new ReflectionClass('I');
  $ni = new ReflectionClass('N\I');
  $ti = new ReflectionClass('T\I');

  var_dump($i->implementsInterface('I'));
  var_dump($ni->implementsInterface('N\I'));
  var_dump($ti->implementsInterface('T\I'));

  var_dump($i->implementsInterface('N\I'));
  var_dump($ti->implementsInterface('N\I'));
  var_dump($ni->implementsInterface('I'));
}
