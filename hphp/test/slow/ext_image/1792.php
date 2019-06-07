<?hh


<<__EntryPoint>>
function main_1792() {
for ($i = 0;
 $i < 100000;
 $i++) {
  $str =  exif_tagname($i);
  if ($str) {
    echo "$i: $str\n";
  }
}
}
