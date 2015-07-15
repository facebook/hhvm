<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function not_in_class(): bool {
  /* HH_FIXME[4061]: static outside class context */
  /* HH_FIXME[4026]: class id unclear when accessing ::class */
  $x = static::class;
  $y = get_called_class();
  return $x == $y;
}
