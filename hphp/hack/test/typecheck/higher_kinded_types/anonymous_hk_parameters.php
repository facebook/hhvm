<?hh // strict


// bad
type Test1<_> = int;

// bad
type Test2<_<T>> = int;

// good
type Test3<T<_>> = int;

// good
type Test4<T1<_,_> > = int;

// good
type Test5<T1<_>, T2<_> > = int;
