<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

class CExplicit implements StringishObject{
  public function __toString(): string {
    return __CLASS__;
  }
}

class CImplicit {
  public function __toString(): string {
    return __CLASS__;
  }
}

interface IImplicit {
  public function __toString(): string;
}

function f1(Stringish $x): string {
  return __FUNCTION__.': '.$x;
}

function f2(): void {
  f1("a boring string");
  $x = "dynamic ";
  $x .= "string";
  f1($x);
  $explicit = new CExplicit();
  f1($explicit);
  $implicit = new CImplicit();
  f1($implicit);
}

function f3(IImplicit $i): void {
  f1($i);
}

trait TStringish {
  public function __toString(): string {
    return __TRAIT__;
  }

  private function foo(): void {
    echo 'foo'.$this;
  }
}

trait TReq {
  require implements StringishObject;

  private function foo(): void {
    echo 'foo'.$this;
  }
}
