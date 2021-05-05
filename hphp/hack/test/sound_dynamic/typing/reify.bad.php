<?hh

class C<<<__NoRequireDynamic>> reify T> {}

function expect_Cdyn(C<dynamic> $c) : void { }

function expect_Cint(C<int> $c) : void { }

function test(C<dynamic> $cd, C<int> $ci) : void {
  expect_Cdyn($ci);
  expect_Cint($cd);
  expect_Cdyn(new C<int>());
  expect_Cint(new C<dynamic>());
}
