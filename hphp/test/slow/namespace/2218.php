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
namespace bar {
  use foo\baz as baz;
  baz\foo();
}
