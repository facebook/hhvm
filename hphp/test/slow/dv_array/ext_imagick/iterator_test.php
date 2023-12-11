<?hh

function row_dump($row, $pixels) :mixed{
  printf("[%d] ", $row);
  if ($pixels === null) {
    var_dump($pixels);
  } else {
    printf("(%d)", count($pixels));
    foreach ($pixels as $pixel) {
      $color = $pixel->getColor(false);
      printf(" #%02x%02x%02x", $color['r'], $color['g'], $color['b']);
    }
    printf("\n");
  }
}
<<__EntryPoint>> function main(): void {
$magick = new Imagick(__DIR__.'/facebook.png');

$iterators = vec[
  ImagickPixelIterator::getPixelIterator($magick),
  ImagickPixelIterator::getPixelRegionIterator($magick, 40, 30, 20, 10),
];

foreach ($iterators as $iterator) {
  for ($i = 0; $i < 5; ++$i) {
    echo "== $i ==\n";
    row_dump($iterator->getIteratorRow(), $iterator->getCurrentIteratorRow());
    row_dump($iterator->getIteratorRow(), $iterator->getPreviousIteratorRow());
    row_dump($iterator->getIteratorRow(), $iterator->getNextIteratorRow());
    row_dump($iterator->getIteratorRow(), $iterator->getNextIteratorRow());

    switch ($i) {
    case 0:
      $iterator->setIteratorRow(5);
      break;
    case 1:
      $iterator->setIteratorFirstRow();
      break;
    case 2:
      $iterator->setIteratorLastRow();
      break;
    case 3:
      $iterator->resetIterator();
      break;
    case 4:
      $iterator->syncIterator();
      break;
    }
  }
}

echo "==DONE==";
}
