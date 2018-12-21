<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class B<T> {
  function __construct() {}
}

class C<T> {
  function __construct<Tc>() {}
}
