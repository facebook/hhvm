<?hh

function count_rows ($pix) :mixed{
  $rows = 0;
  foreach ($pix as $r) {
    $rows++;
  }
  return $rows;
}

function count_objects ($pix) :mixed{
  $objects = 0;
  foreach ($pix as $r) {
    foreach ($r as $o) {
      $objects++;
    }
  }
  return $objects;
}
<<__EntryPoint>> function main(): void {
$im = new Imagick ('magick:rose');
$it1 = new ImagickPixelIterator ($im);

$it2 = ImagickPixelIterator::getPixelIterator ($im);
echo (count_rows ($it1) == count_rows ($it2) ? "match" : "no") . PHP_EOL;
echo (count_objects ($it1) == count_objects ($it2) ? "match" : "no") . PHP_EOL;

$it1->newPixelIterator (new Imagick ('magick:rose'));

echo 'done' . PHP_EOL;
}
