<?hh


<<__EntryPoint>>
function main_preg_replace_named_subpat() :mixed{
$content = 'b';
$pattern = '/(?P<a>.*)/';

$count = -1;
preg_replace_callback(
  $pattern,
  $m ==> {
    var_dump($m);
    var_dump(is_darray($m));
  },
  $content,
  -1,
  inout $count
);
}
