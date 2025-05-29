<?hh
<<file: __EnableUnstableFeatures('polymorphic_lambda')>>

class A {}

abstract class B {
   abstract const type TA as A;
}

class C<T1 as A, T2 as B with { type TA as T1 }> {}

function test(): void {
  $_ = function<T1 as A, T2 as B with { type TA as T1 }>(C<T1, T2> $_): int ==> {
    // This is a comment which should be preserved even when it exceeds column 80
    // and then wraps around
    return 0;
  };


  $_ = function<T1 as A, T2 as B with { type TA as T1 }>(C<T1, T2> $_): int use () {
    // This is a comment which should be preserved even when it exceeds column 80
    // and then wraps around
    return 0;
  };

}
