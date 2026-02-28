<?hh


<<__EntryPoint>>
function main_preg_replace_named_subpat() :mixed{
$content = 'b';
$pattern = '/(?P<a>.*)/';

$count = -1;
preg_replace_callback($pattern, function ($m) { var_dump($m); }, $content, -1, inout $count);
}
