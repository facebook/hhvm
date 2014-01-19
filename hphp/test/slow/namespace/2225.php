<?php

namespace {
  function foo() {
 var_dump(__NAMESPACE__);
}
}
namespace B {
  function foo() {
 var_dump(__NAMESPACE__);
}
}
namespace B {
  call_user_func('foo');
}
