<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class :a {
  attribute int i;
  attribute string s @required;

  public function test(dynamic $d): void {
    hh_show($this->:i);
    hh_show($this->:s);
  }
}

function ftest(dynamic $d): void {
  $a = <a i={$d} s={$d}/>;
}
