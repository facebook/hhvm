<?hh //strict

class MyExn extends Exception {}

function might_throw(): void {}

function f(): void {
  try {
    might_throw();
  } catch (Exception $e) {
    throw new Exception("Error Processing Request", 1);
  } catch (MyExn $e) {
  }
}
