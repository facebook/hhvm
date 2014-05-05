<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

// A simple duck typing example

class DoesRender {

  public function render(): string {
    return "";
  }
}

class MyClass1 {

  public function otherStuff(): string {
    return "I know how to do other stuff too!";
  }

  public function render(string $x = ""): string {
    return "Yoohoo I know how to render";
  }
}

function test(DoesRender $obj): string {
  return $obj->render();
}

function main(): void {
  $my_string = test(new MyClass1()) ;
}
