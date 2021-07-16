<?hh

class MyC {
  public string $prop = "property";
  public static string $glob = "global";
}

function controlled_good_ops(MyC $c)[controlled]: void {
  $c->prop = "ok";
}

function controlled_bad_ops()[controlled]: void {
  echo "bad"; // ERROR: missing `IO`
  MyC::$glob = "bad"; // ERROR: missing `AccessGlobals`
}
