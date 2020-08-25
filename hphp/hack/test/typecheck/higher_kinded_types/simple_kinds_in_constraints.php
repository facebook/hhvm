<?hh // strict




class C<T, T2<_>> {}


// kind mismatch here
function test<TX as C<int, C>>() : void {}
