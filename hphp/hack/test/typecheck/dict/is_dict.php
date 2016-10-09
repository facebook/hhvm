<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test_is_dict<Tk as arraykey, Tv>(Indexish<Tk, Tv> $m): ?dict<Tk, Tv> {
  if (is_dict($m)) {
    return $m;
  }
  return null;
}
