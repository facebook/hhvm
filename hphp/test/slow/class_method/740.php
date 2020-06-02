<?hh

<<__EntryPoint>>
function entrypoint_740(): void {

  if (__hhvm_intrinsics\launder_value(true)) {
    include '740-1.inc';
  } else {
    include '740-2.inc';
  }

  include '740.d.inc';

  $obj = new d;
  foreach ($obj as $k => $v) {
  }
}
