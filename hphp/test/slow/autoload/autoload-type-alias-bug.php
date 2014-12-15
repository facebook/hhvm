<?hh
function autoload_miss($str1, $str2) {
  echo "Failure handler called: $str1 $str2\n";
}
HH\autoload_set_paths(
  array(
    'class' => array(),
    'constant' => array(),
    'function' => array(),
    'failure' => 'autoload_miss',
    'type' => array('foo' => 'autoload-type-alias-bug-2.inc'),
  ),
  __DIR__.'/'
);
include 'autoload-type-alias-bug-1.inc';
echo "Done\n";
