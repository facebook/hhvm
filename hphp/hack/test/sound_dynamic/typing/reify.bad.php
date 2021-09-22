<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

class C<<<__NoRequireDynamic>> reify T> {}

function expect_Cdyn(C<dynamic> $c) : void { }

function expect_Cint(C<int> $c) : void { }

function test(C<dynamic> $cd, C<int> $ci) : void {
  $ci upcast C<dynamic>;
  $cd upcast C<int>;
  new C<int>() upcast C<dynamic>;
  new C<dynamic>() upcast C<int>;
}
