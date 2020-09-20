<?hh

<<__Const>>
class C {
  public function __construct(public int $i) {}
  public function __wakeup(): void {
    $this->i++;
  }
}

<<__EntryPoint>>
function test() {
  $c = new C(1);
  var_dump($c);
  apc_store(__FILE__, $c);
  // Need to fetch the value twice because object are serialized on first store,
  // APCObjects are created on the first fetch and used on the second fetch.
  $c2 = __hhvm_intrinsics\apc_fetch_no_check(__FILE__);
  var_dump($c2);
  // TODO(alexeyt) note that we get an object on which __wakeup has run twice!
  $c3 = __hhvm_intrinsics\apc_fetch_no_check(__FILE__);
  var_dump($c3);
  try {
    $c3->i++;
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  var_dump($c3);
}
