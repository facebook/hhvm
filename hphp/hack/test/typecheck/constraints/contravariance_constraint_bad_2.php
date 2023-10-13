<?hh
// Copyright 2004-present Facebook. All Rights Reserved.
// Trying to replicate one of the contravariant constraint issues in www
// This is the last one that shows up in www

namespace NAC;

// If this is covariant or contravariant then the constraint on DDD doesn't get checked!
interface CONTRA<-T> {
  public function CallWith(T $arg): void;
}

// It's this constraint that fails to be satisfied
class CCC<-T as AAA> implements CONTRA<T> {

  public function __construct(private (function(T): void) $myfun) {}

  public function CallWith(T $arg): void {
    $f = $this->myfun;
    ($f)($arg);
  }
}

class AAA {
  public function foo(): void {
    echo "AAA::foo";
  }
}

class BBB {
  public function bar(): void {
    echo "BBB::bar";
  }
}

class DDD<T as BBB> {
  public function ddd(): CONTRA<T> {
    $c = new CCC(
      function(AAA $arg) {
        $arg->foo();
      },
    );
    return $c;
  }
}

class MMM {
  public static function Main(): void {
    $b = (new DDD())->ddd();
    $b->CallWith(new BBB());
  }
}
//MMM::Main();
