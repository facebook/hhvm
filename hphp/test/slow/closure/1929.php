<?hh

function h() :mixed{
  return array_filter(vec[1, 2, 3],
                      function($e) {
 return !($e & 1);
 }
);
}

<<__EntryPoint>>
function main_1929() :mixed{
h();
var_dump(h());
}
