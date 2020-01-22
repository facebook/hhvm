<?hh

<<__ProvenanceSkipFrame>>
function test($x) {
  echo "varray: ";
  var_dump(HH\get_provenance(varray($x)));
  echo "darray: ";
  var_dump(HH\get_provenance(darray($x)));
  echo "vec:    ";
  var_dump(HH\get_provenance(vec($x)));
  echo "dict:   ";
  var_dump(HH\get_provenance(dict($x)));
}

<<__EntryPoint>>
function main() {
  $arrs = __hhvm_intrinsics\launder_value(dict[
                                            "vec" => vec[1, 2, 3],
                                            "varray" => varray[1, 2, 3],
                                            "dict" => dict[42 => "hello"],
                                            "darray" => darray[42 => "hello"],
                                          ]);

  foreach ($arrs as $name => $a) {
    echo "\n--------------------\n" . $name . "\n";
    test($a);
  }
}
