<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function Bar<Tv as Tu, Tu as Tv>(Tv $x): Tu {
  if ($x !== null) {
    echo 'hey';
    return $x;
  }
  return $x;
}
