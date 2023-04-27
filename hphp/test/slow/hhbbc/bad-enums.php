<?hh

class A {}

enum BadEnum1 : string {
  use A;
  VAL1 = "VAL1";
  VAL2 = "VAL2";
  VAL3 = "VAL3";
}

enum BadEnum2 : string {
  use MissingTrait;
  VAL1 = "VAL1";
  VAL2 = "VAL2";
  VAL3 = "VAL3";
}

newtype Alias1 = A;

enum BadEnum3 : Alias1 {
  VAL1 = 1;
  VAL2 = 2;
  VAL3 = 3;
}

enum BadEnum4 : BadEnum3 {
  VAL4 = 4;
  VAL5 = 5;
}

newtype Alias2 = BadEnum4;

enum BadEnum5 : bool {
  use MissingEnum;
  VAL1 = 1;
  VAL2 = 2;
  VAL3 = 3;
}

newtype Alias3 = BadEnum5;

enum BadEnum6 : Alias3 {
  VAL4 = 4;
}

newtype Alias4 = BadEnum6;

function func1(BadEnum1 $e) {
  var_dump($e);
}
function func2(BadEnum2 $e) {
  var_dump($e);
}
function func3(Alias2 $e) {
  var_dump($e);
}
function func4(Alias4 $e) {
  var_dump($e);
}

<<__EntryPoint>>
function main() {
  func1("hi");
  func2("bye");
  func3(1);
  func4(2);
}
