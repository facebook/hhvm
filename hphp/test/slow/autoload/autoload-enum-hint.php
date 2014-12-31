<?hh
function autoload_miss($str1, $str2) {
  echo "Failure handler called: $str1 $str2\n";
}
HH\autoload_set_paths(
  array(
    'class' => array('foo' => 'autoload-enum-hint-2.inc',
                     'bar' => 'autoload-enum-hint-3.inc',
                     'baz' => 'autoload-enum-hint-4.inc'),
    'constant' => array(),
    'function' => array(),
    'failure' => 'autoload_miss',
    'type' => array(),
  ),
  __DIR__.'/'
);
include 'autoload-enum-hint-1.inc';
