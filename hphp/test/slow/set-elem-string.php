<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  private string $str = 'abcdef';
  public function add($k, $c) :mixed{
    $this->str[$k] = $c;
  }
}

<<__EntryPoint>> function test(): void {
  $c = new C();
  $c->add(0, 'z');
  $c->add(0, 'z');
  var_dump(__hhvm_intrinsics\launder_value('z'));
}
