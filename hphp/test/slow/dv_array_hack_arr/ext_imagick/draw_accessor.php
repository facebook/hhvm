<?hh
<<__EntryPoint>> function main(): void {
$draw = new ImagickDraw;

// clip
$draw->setClipRule(Imagick::FILLRULE_EVENODD);
var_dump($draw->getClipRule() === Imagick::FILLRULE_EVENODD);

$draw->setClipUnits(10);
var_dump($draw->getClipUnits());

// fill
$draw->setFillColor('yellow');
var_dump($draw->getFillColor()->getColor());

$draw->setFillOpacity(0.5);
printf("%.2f\n", $draw->getFillOpacity());

$draw->setFillRule(Imagick::FILLRULE_NONZERO);
var_dump($draw->getClipRule() === Imagick::FILLRULE_NONZERO);

// gravity
$draw->setGravity(Imagick::GRAVITY_SOUTHEAST);
var_dump($draw->getGravity() === Imagick::GRAVITY_SOUTHEAST);

// stroke
$draw->setStrokeAntialias(false);
var_dump($draw->getStrokeAntialias());

$draw->setStrokeColor(new ImagickPixel('#F02B88'));
var_dump($draw->getStrokeColor()->getColor());

$draw->setStrokeDashArray(vec[1, 2, 3]);
var_dump($draw->getStrokeDashArray());

$draw->setStrokeDashOffset(-1.0);
var_dump($draw->getStrokeDashOffset());

$draw->setStrokeLineCap(Imagick::LINECAP_SQUARE);
var_dump($draw->getStrokeLineCap() === Imagick::LINECAP_SQUARE);

$draw->setStrokeLineJoin(Imagick::LINEJOIN_BEVEL);
var_dump($draw->getStrokeLineJoin() === Imagick::LINEJOIN_BEVEL);

$draw->setStrokeMiterLimit(3);
var_dump($draw->getStrokeMiterLimit());

$draw->setStrokeOpacity(0.9);
printf("%.2f\n", $draw->getStrokeOpacity());

$draw->setStrokeWidth(1.2);
printf("%.2f\n", $draw->getStrokeWidth());

// text
$draw->setTextAlignment(Imagick::ALIGN_CENTER);
var_dump($draw->getTextAlignment() === Imagick::ALIGN_CENTER);

$draw->setTextAntialias(false);
var_dump($draw->getTextAntialias());

$draw->setTextDecoration(Imagick::DECORATION_LINETROUGH);
var_dump($draw->getTextDecoration() === Imagick::DECORATION_LINETROUGH);

$draw->setTextEncoding('UTF-8');
var_dump($draw->getTextEncoding());

$draw->setTextUnderColor('cyan');
var_dump($draw->getTextUnderColor()->getColor());
}
