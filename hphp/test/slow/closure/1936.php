<?hh

function test($x) :mixed{
  $s_path = serialize($x);
  $filter = function ($rel) use ($s_path) {
    return $s_path;
  }
;
  var_dump($filter(0));
}

<<__EntryPoint>>
function main_1936() :mixed{
test('hello');
test(dict[0 => 1, 1 => 2,'foo'=>'bar']);
}
