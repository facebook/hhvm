<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function null_subtype_option_nothing(): ?nothing {
  return null;
}

function option_nothing_subtype_null(?nothing $x): null {
  return $x;
}
