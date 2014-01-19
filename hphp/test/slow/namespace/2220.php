<?php

namespace {
  function foo() {
 var_dump(__NAMESPACE__);
}
}
namespace B {
}
namespace B {
  foo();
}
