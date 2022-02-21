<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.


function expect_string(~string $_): void {}

<<__SupportDynamicType>>
interface ExBox {}

<<__SupportDynamicType>>
class Box<T as dynamic > implements ExBox {
  public function __construct(public ~T $data)[] {}
}

<<__SupportDynamicType>>
class IBox extends Box<int> {
  public function add(~int $x)[write_props]: void {
    $this->data = $this->data + $x;
  }
}

enum class E: ~ExBox {
   ~Box<string> A = new Box('bli');
}

<<__SupportDynamicType>>
function e<T as dynamic >(~HH\MemberOf<E, Box<T>> $param): ~T {
  return $param->data;
}

<<__SupportDynamicType>>
function testit(): void {
  expect_string(e(E::A));
}
