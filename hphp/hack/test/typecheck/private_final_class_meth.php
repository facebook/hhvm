<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

abstract class Base {
  final private function privateFinal(): string {
    return 'Base';
    }
}
final class Child extends Base {
    private function privateFinal(): string {
        return 'Child';
  }
}
<<__EntryPoint>>
function main():void {
  $x = new Child();
}
