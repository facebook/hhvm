<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function checkSet<Tv as arraykey>(
    Traversable<Tv> $set,
  ): Container<Tv> {
  if (!($set is Container<_>)) {
    $set = keyset($set);
  } else {
  }
  return $set;
}
