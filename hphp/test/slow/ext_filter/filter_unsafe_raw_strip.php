<?php

// From https://github.com/facebook/hhvm/issues/2337

var_dump(
  filter_var(
    "a\ta",
    FILTER_UNSAFE_RAW,
    FILTER_FLAG_STRIP_LOW | FILTER_FLAG_NO_ENCODE_QUOTES
  )
);
