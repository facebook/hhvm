<?hh

function identity($arr) {
  return $arr;
}

var_dump(dict(darray(identity(dict["foo" => "bar"]))));
