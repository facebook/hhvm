<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

namespace Foo {
  use function HH\set_bytes_int64;

  function test_primitive():void {
    $x = '12345678';
    // legal => SetRangeM
    \HH\set_bytes_int64($x, 0, 0x4142434445464748);
    // illegal => FCallFuncD "Foo\\HH\\set_bytes..."
    HH\set_bytes_int64($x, 0, 0x4142434445464748);
    // legal => SetRangeM
    set_bytes_int64($x, 0, 0x4142434445464748);
  }
}

namespace {

  function test_primitive_HH():void {
    $x = '12345678';
    // legal => SetRangeM
    \HH\set_bytes_int64($x, 0, 0x4142434445464748);
    // legal => SetRangeM
    HH\set_bytes_int64($x, 0, 0x4142434445464748);
  }
}

namespace {
  use Foo as X;
  use function HH\set_bytes_int64;

  function test_primitive_outside():void {
    $x = '12345678';
    // illegal => FCallFuncD "Foo\\HH\\set_bytes..."
    X\HH\set_bytes_int64($x, 0, 0x4142434445464748);
    // legal => SetRangeM
    set_bytes_int64($x, 0, 0x4142434445464748);
  }

}
