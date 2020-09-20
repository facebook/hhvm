<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

trait T1 {}
trait T2 {
  require extends T1;
}
