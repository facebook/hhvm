<?hh

function identity($arr) :mixed{
  return $arr;
}


<<__EntryPoint>>
function main_in_place() :mixed{
var_dump(dict(darray(identity(dict["foo" => "bar"]))));
}
