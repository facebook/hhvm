<?hh

<<__ProvenanceSkipFrame>>
function map($v, $f) {
  $ret = varray[];
  foreach($v as $x) {
    $ret[] = $f($x);
  }
  return $ret;
}

<<__EntryPoint>>
function main(): void {
  $a = varray[1, 2, 3];
  $e = varray[];

  $results = varray[];
  $results[] = map($a, $x ==> $x + 1);
  $results[] = map($a, $x ==> $x + 2);
  $results[] = map($e, $x ==> $x + 1);
  $results[] = map($e, $x ==> $x + 2);

  foreach($results as $v) {
    var_dump(HH\get_provenance($v));
  }
}
