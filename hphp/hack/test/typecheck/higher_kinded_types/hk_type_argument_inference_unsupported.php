<?hh // strict


function test<T1, T2<T3>>() : void {}


function bad1() : void {
  test<_,_>(); //error in second case: cannot provide _ for HK type
}

function bad2() : void {
  test(); // cannot omit args when function expects HK args.
}
