<?hh

class Box<T> {}

function unsurfaced_exception_lambda_good(Box<int> $_): void {
  try {
    42;
  } catch (Exception $ex) {
    () ==> $ex;
  }
}
