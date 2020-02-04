<?hh

<<__EntryPoint>>
function main() {
  $x = json_decode('{"foo": 2, "bar": 3, "baz": 5}',
                   true, 512, JSON_FB_HACK_ARRAYS);
  $x = HH\tag_provenance_here($x);
  var_dump($x);
}
