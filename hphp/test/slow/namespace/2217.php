<?php

namespace foo\baz {
  function foo() {
 var_dump(__NAMESPACE__);
}
}
namespace bar\baz {
  function foo() {
 var_dump(__NAMESPACE__);
}
}
namespace {
  use foo\baz as baz;
  baz\foo();
}
