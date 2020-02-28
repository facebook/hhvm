<?hh
function autoload_miss($str1, $str2) {
  echo "Failure handler called: $str1 $str2\n";
}

<<__EntryPoint>>
function main_autoload_enum_hint_deprecated() {
fb_autoload_map(
  darray[
    'class' => darray['foo' => 'autoload-enum-hint-2.inc',
                     'bar' => 'autoload-enum-hint-3.inc',
                     'baz' => 'autoload-enum-hint-4.inc'],
    'constant' => varray[],
    'function' => varray[],
    'failure' => 'autoload_miss',
    'type' => varray[],
  ],
  __DIR__.'/'
);
include 'autoload-enum-hint-1.inc';
}
