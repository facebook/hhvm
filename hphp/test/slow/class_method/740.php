<?hh

if (__hhvm_intrinsics\launder_value(true)) {
  include '740-1.inc';
} else {
  include '740-2.inc';
}
class d extends c {
  public function rewind() {
    var_dump('rewinding');
  }
}
$obj = new d;
foreach ($obj as $k => $v) {
}
