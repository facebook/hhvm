<?php

function get_coord($ox, $oy, $rx, $ry, $arg) {
  return array(
    'x' => $ox + $rx * cos(deg2rad($arg)),
    'y' => $oy + $ry * sin(deg2rad($arg))
  );
}

function draw_pie(&$canvas, $ox, $oy, $r, $sd, $ed, $color) {
  $draw = new ImagickDraw;

  $draw->setFillColor($color);
  $draw->setStrokeColor($color);
  $draw->setStrokeWidth(1);

  $draw->arc($ox - $r, $oy - $r, $ox + $r, $oy + $r, $sd, $ed);
  $draw->polygon(array(
    array('x' => $ox, 'y' => $oy),
    get_coord($ox, $oy, $r, $r, $sd),
    get_coord($ox, $oy, $r, $r, $ed)
  ));

  $canvas->drawImage($draw);
}

function test_annotation(&$canvas) {
  $draw = new ImagickDraw;
  $font = __DIR__.'/php_imagick_tests/anonymous_pro_minus.ttf';

  $draw->setFont($font);
  $draw->setFontSize(40);
  $draw->annotation(50, 440, 'Yukkuri shiteitte ne!!!');

  $canvas->drawImage($draw);
}

function test_composite(&$canvas) {
  $draw = new ImagickDraw;
  $fblogo = new Imagick(__DIR__.'/facebook.png');

  $draw->composite(Imagick::COMPOSITE_ATOP, 1, 1, 175, 50, $fblogo);

  $canvas->drawImage($draw);
}

function test_path(&$canvas) {
  $draw = new ImagickDraw;

  $draw->setFillColor('transparent');
  $draw->setStrokeColor('blue');
  $draw->setStrokeWidth(4);

  $draw->pathStart();
  $draw->pathMoveToAbsolute(300, 10);
  $draw->pathLineToHorizontalRelative(50);
  $draw->pathLineToVerticalRelative(50);
  $draw->pathEllipticArcRelative(40, 30, 30, false, true, 60, 10);
  $draw->pathLineToAbsolute(444, 22);
  $draw->pathFinish();

  $draw->pathStart();
  $draw->pathMoveToAbsolute(480, 30);
  $draw->pathCurveToAbsolute(520, 40, 560, 10, 600, 20);
  $draw->pathCurveToQuadraticBezierRelative(30, 10, -60, 80);
  $draw->pathCurveToQuadraticBezierSmoothRelative(20, -50);
  $draw->pathClose();
  $draw->pathFinish();

  $canvas->drawImage($draw);
}

function test_shape(&$canvas) {
  $draw = new ImagickDraw;

  $draw->setFillColor('transparent');
  $draw->setStrokeColor('#F02B88');
  $draw->setStrokeWidth(9);

  $draw->translate(200, 100);
  $draw->rectangle(-50, -50, 50, 50);

  $draw->translate(200, 100);
  $draw->ellipse(0, 0, 100, 80, 0, 360);

  $draw->skewX(-30);

  $draw->translate(200, 100);
  $draw->circle(0, 0, 50, 50);

  $canvas->drawImage($draw);
}

$canvas = new Imagick;
$canvas->newImage(640, 480, 'white');

draw_pie($canvas, 200, 200, 100, 0, 45, 'red');
draw_pie($canvas, 200, 200, 100, 45, 125, 'green');
draw_pie($canvas, 200, 200, 100, 125, 225, 'blue');
draw_pie($canvas, 200, 200, 100, 225, 300, 'cyan');
draw_pie($canvas, 200, 200, 100, 300, 360, 'orange');

test_shape($canvas);
test_path($canvas);
test_composite($canvas);
test_annotation($canvas);

// $actual_file = __DIR__.'/draw_example.out.png';
$expected_file = __DIR__.'/draw_example.png';
// $canvas->writeImage($actual_file);

$bg = new ImagickPixel('white');
$actual_iterator = new ImagickPixelIterator($canvas);
$expected_iterator =  new ImagickPixelIterator(new Imagick($expected_file));

list($total, $same, $similar, $diff) = array(0, 0, 0, 0);

$actual_iterator->rewind();
$expected_iterator->rewind();
while ($actual_iterator->valid() && $expected_iterator->valid()) {
  if ($expected_iterator->key() !== $expected_iterator->key()) {
    var_dump(array(
      'actual' => $expected_iterator->key(),
      'expected' => $expected_iterator->key()
    ));
  }
  $actual_pixels = $actual_iterator->current();
  $expected_pixels = $expected_iterator->current();
  foreach (array_map(null, $actual_pixels, $expected_pixels) as $value) {
    list($actual, $expected) = $value;
    if ($actual->isPixelSimilar($bg, 0.01) &&
        $expected->isPixelSimilar($bg, 0.01)) {
      continue;
    }
    ++$total;
    if ($actual->isPixelSimilar($expected, 0.01)) {
      ++$same;
    } else if ($actual->isPixelSimilar($expected, 0.2)) {
      ++$similar;
    } else {
      ++$diff;
    }
  }
  $actual_iterator->next();
  $expected_iterator->next();
}

var_dump(array(
  'total'   => $total,
  'same'    => $same,
  'similar' => $similar,
  'diff'    => $diff
));
var_dump($total > 55000 && $total < 65000);
var_dump($same > $total * 0.92);
var_dump($diff < $total * 0.04);
var_dump($actual_iterator->valid());
var_dump($expected_iterator->valid());
