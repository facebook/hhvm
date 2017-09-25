<?hh

<<__MockClass>>
class MyWaitHandle extends WaitHandle {
  public function __construct() {
    echo "Ha ha!\n";
  }
}

try {
  $wh = new MyWaitHandle;
} catch (Exception $e) {
  echo "Exception: ", $e->getMessage(), "\n";
}
