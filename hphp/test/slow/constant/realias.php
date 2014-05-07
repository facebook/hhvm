<?php

namespace foo\bar {
  const baz = 42;
}

namespace other {
  const baz = 100;
}

namespace {
  use const foo\bar\baz;
  use const other\baz;

  var_dump(baz);
}
