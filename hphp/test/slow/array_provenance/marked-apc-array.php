<?hh

class C {}

<<__EntryPoint>>
function main() {
  $x = varray[new C()];
  $y = HH\array_mark_legacy(varray[new C()]);
  apc_store('x', $x);
  apc_store('y', $y);

  $success = null;
  $x = apc_fetch('x', inout $success);
  $y = apc_fetch('y', inout $success);
  print('$x = '.json_encode($x).' @ '.HH\get_provenance($x)."\n");
  print('$y = '.json_encode($y).' @ '.HH\get_provenance($y)."\n");

  apc_delete('x');
  apc_delete('y');
  print("Done!\n");
}
