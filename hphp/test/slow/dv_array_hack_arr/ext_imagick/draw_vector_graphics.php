<?php

$draw = new ImagickDraw;

$svg = <<<SVG
<svg widthxmlns="http://www.w3.org/2000/svg" version="1.1">
  <rect width="300"
        height="100"
        style="fill:rgb(0,0,255);stroke-width:1;stroke:rgb(0,0,0)" />
</svg>
SVG;

$draw->setVectorGraphics($svg);
var_dump($draw->getVectorGraphics());

try {
  $draw->setVectorGraphics('invalid-svg');
} catch (Exception $ex) {
  var_dump('setVectorGraphics');
}
