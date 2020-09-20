<?hh
<<__EntryPoint>>
function main_entry(): void {

  var_dump(json_encode(""));
  var_dump(json_encode(NULL));
  var_dump(json_encode(TRUE));

  var_dump(json_encode(darray[""=>""]));
  var_dump(json_encode(varray[varray[1]]));
  var_dump(json_encode(varray[]));

  var_dump(json_encode(darray[""=>""], JSON_FORCE_OBJECT));
  var_dump(json_encode(varray[varray[1]], JSON_FORCE_OBJECT));
  var_dump(json_encode(varray[], JSON_FORCE_OBJECT));

  var_dump(json_encode(1));
  var_dump(json_encode("руссиш"));


  echo "Done\n";
}
