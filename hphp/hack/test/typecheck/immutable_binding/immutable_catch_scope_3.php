<?hh // experimental

class ExA extends Exception {}
class ExB extends Exception {}

function func(): void {}

function foo(): void {
  try {
    func();
  } catch (ExA e) {
    let a = "A";
  } catch (ExB e) {
    echo a; // error
  }
}
