<?hh //strict

function might_throw(): void {}

function f(): void {
  try {
    might_throw();
  } catch (Exception $ex) {
    try {
      might_throw();
    } catch (Exception $exe) {
    }
    throw $ex;
  } finally {
  }
}
