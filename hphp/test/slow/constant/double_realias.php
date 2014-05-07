<?php

namespace foo {
  const bar = 'foo\bar';
  const BAR = 'foo\BAR';
}

namespace {
  use const foo\bar as foobar;
  use const foo\bar as FOOBAR;

  use const foo\BAR as fooBAR;
  use const foo\BAR as FOObar;

  var_dump(foo\bar);
  var_dump(foo\BAR);

  var_dump(foobar);
  var_dump(FOOBAR);
  var_dump(fooBAR);
  var_dump(FOObar);
}
