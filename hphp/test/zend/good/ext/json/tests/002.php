<?hh
<<__EntryPoint>>
function main_entry(): void {

  var_dump(json_encode(""));
  var_dump(json_encode(NULL));
  var_dump(json_encode(TRUE));

  var_dump(json_encode(dict[""=>""]));
  var_dump(json_encode(vec[vec[1]]));
  var_dump(json_encode(vec[]));

  var_dump(json_encode(dict[""=>""], JSON_FORCE_OBJECT));
  var_dump(json_encode(vec[vec[1]], JSON_FORCE_OBJECT));
  var_dump(json_encode(vec[], JSON_FORCE_OBJECT));

  var_dump(json_encode(1));
  var_dump(json_encode("руссиш"));


  echo "Done\n";
}
