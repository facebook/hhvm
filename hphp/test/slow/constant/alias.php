<?php

namespace foo\bar {
  const baz = 42;
}

namespace {
  use const foo\bar\baz;

  var_dump(baz);
}
