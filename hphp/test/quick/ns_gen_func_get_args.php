<?php
namespace foo {
  function bar_namespaced() {
    $args = func_get_args();
    var_dump($args);
    yield 42;
  }
}

namespace {
  foo\bar_namespaced(1, 2, 3)->next();
}

?>
