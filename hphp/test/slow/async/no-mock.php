<?hh

<<__MockClass>>
class MyWaitHandle extends Awaitable {
  public function __construct() {
    echo "Ha ha!\n";
  }
}


<<__EntryPoint>>
function main_no_mock() :mixed{
try {
  $wh = new MyWaitHandle;
} catch (Exception $e) {
  echo "Exception: ", $e->getMessage(), "\n";
}
}
