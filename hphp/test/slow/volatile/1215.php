<?hh


<<__EntryPoint>>
function main_1215() :mixed{
for ($i = 0;
 $i < 4;
 $i++) {
  if (defined('CON')) {
    var_dump(CON);
  }
 else {
    echo "CON does not exists\n";
  }
}
for ($i = 0;
 $i < 4;
 $i++) {
  if ($i > 1 && !class_exists('bar')) {
    include '1215.inc';
  }
  if (class_exists('bar')) {
    $a = new bar;
  }
 else {
    echo "bar does not exists\n";
  }
}
}
