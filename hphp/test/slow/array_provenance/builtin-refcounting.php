<?hh

class C {}

<<__EntryPoint>>
function main() {
  $x = varray[new C()];
  $y = HH\array_mark_legacy($x);
  print('legacy($x) = '.(int)HH\is_array_marked_legacy($x)."\n");
  print('legacy($y) = '.(int)HH\is_array_marked_legacy($y)."\n");

  $x = varray[new C()];
  $y = HH\tag_provenance_here($x);
  print('provenance($x) = '.HH\get_provenance($x)."\n");
  print('provenance($y) = '.HH\get_provenance($y)."\n");
}
