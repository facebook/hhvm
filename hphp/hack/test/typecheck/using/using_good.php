<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class DBHandle {
  public function __construct(private string $id) { }
}
function test_it():void {
  using ($user_db = new DBHandle('users')) {
  }

  using ($x = new DBHandle('x'), $y = new DBHandle('y')) {
  }
}
function test_function_scope():void {
  using $user_db = new DBHandle('users');
  using ($x = new DBHandle('x'), $y = new DBHandle('y'));
  echo "e";
}
