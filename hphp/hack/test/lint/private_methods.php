<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class TestDeadPrivateMethod {
  // sanity, public/private/protected
  public function publicMethodOuter(): string {
    return $this->protectedMethod() . $this->publicMethodInner();
  }

  public function publicMethodInner(): string {
    return $this->livePrivateMethod();
  }

  protected function protectedMethod(): string {
    return "Never doubt I love";
  }

  private function livePrivateMethod(): string {
    return "I am alive";
  }

  private function deadPrivateMethodOuter(): string {
    return $this->deadPrivateMethodInner();
  }

  private function deadPrivateMethodInner(): string {
    return "Life's but a walking shadow, signifying nothing.";
  }

  // recursion
  private function deadPrivateMethodSelfRecursion(
    int $counter,
  ): void {
    if ($counter > 0) {
      $this->deadPrivateMethodSelfRecursion($counter - 1);
    }
  }

  private function deadPrivateMethodMutualRecursion1(
    int $counter,
  ): void {
    if ($counter > 0) {
      $this->deadPrivateMethodMutualRecursion2($counter - 1);
    }
  }

  private function deadPrivateMethodMutualRecursion2(
    int $counter,
  ): void {
    if ($counter > 0) {
      $this->deadPrivateMethodMutualRecursion1($counter - 2);
    }
  }

  // static methods
  static public function staticPublicMethod(): float {
    return self::liveStaticPrivateMethod();
  }

  static private function liveStaticPrivateMethod(): float {
    return 3.1415;
  }

  static private function deadStaticPrivateMethod(): float {
    return 2.7182;
  }

  // abstract methods
  abstract static public function abstractStaticPublicMethod(): float;
}


class FinalTestDeadPrivateMethod extends TestDeadPrivateMethod {
  <<__Override>>
  final static public function abstractStaticPublicMethod(
  ): float {
    return self::staticPublicMethod();
  }
}


trait SomeTrait {
  private function liveTraitMethod(): string {
    return "Trait methods are always assumed alive, because a class that " .
           "uses the trait can access privates methods defined in the trait ".
           "but we don't know about the class in Zoncolan call graph ";
  }
}
