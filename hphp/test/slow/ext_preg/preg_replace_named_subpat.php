<?hh


<<__EntryPoint>>
function main_preg_replace_named_subpat() {
$content = 'b';
$pattern = '/(?P<a>.*)/';

preg_replace_callback(
  $pattern,
  $m ==> {
    var_dump($m);
    var_dump(is_darray($m));
  },
  $content,
);
}
