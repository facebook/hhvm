<?php

namespace const_case {
  const lower = 'lower';
  const UPPER = 'UPPER';
}

namespace {
  use const const_case\lower as LOWER;
  use const const_case\UPPER as upper;

  use const const_case\lower;
  use const const_case\UPPER;

  var_dump(lower);
  var_dump(LOWER);

  var_dump(upper);
  var_dump(UPPER);
}
