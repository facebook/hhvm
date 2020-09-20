<?hh

function identity($arr) {
  return $arr;
}


<<__EntryPoint>>
function main_in_place() {
var_dump(dict(darray(identity(dict["foo" => "bar"]))));
}
