<?hh
// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

final class TheProblem {
  public static function testDemo(): void {
    $demo = new Demo();
    $demo->asEnt();
  }
}

final class Demo {
  use TParentDemo;

  public function asEnt(): void {
    echo('in TParentDemo');
  }
}

trait TParentDemo {
  require class Demo;
  use TChild1Demo;
  use TChild2Demo;
}

trait TChild1Demo {
  public function asEnt(): void {
    echo('in TChild1Demo');
  }
}

trait TChild2Demo {
  public function asEnt(): void {
    echo('in TChild2Demo');
  }
}
