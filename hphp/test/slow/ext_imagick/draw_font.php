<?php

$draw = new ImagickDraw;

$font_path = __DIR__.'/php_imagick_tests/anonymous_pro_minus.ttf';
$draw->setFont($font_path);
var_dump($draw->getFont() === $font_path);

try {
  $draw->setFont(">_<");
} catch (Exception $ex) {
  var_dump("setFont");
}

$draw->setFontSize(12);
var_dump($draw->getFontSize());

$draw->setFontStretch(Imagick::STRETCH_SEMIEXPANDED);
var_dump($draw->getFontStretch() === Imagick::STRETCH_SEMIEXPANDED);

$draw->setFontStyle(Imagick::STYLE_ITALIC);
var_dump($draw->getFontStyle() === Imagick::STYLE_ITALIC);

$draw->setFontWeight(500);
var_dump($draw->getFontWeight());

try {
  $draw->setFontWeight(1000);
} catch (ImagickDrawException $ex) {
  var_dump($ex->getMessage());
}

