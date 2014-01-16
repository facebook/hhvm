<?php
$cwd = dirname(__FILE__);
$font = "$cwd/Tuffy.ttf";
$delta_t = 360.0 / 16; # Make 16 steps around
$g = imagecreate(800, 800);
$bgnd  = imagecolorallocate($g, 255, 255, 255);
$black = imagecolorallocate($g, 0, 0, 0);
$x = 100;
$y = 0;
$cos_t = cos(deg2rad($delta_t));
$sin_t = sin(deg2rad($delta_t));
for ($angle = 0.0; $angle < 360.0; $angle += $delta_t) {
  $bbox = imagettftext($g, 24, $angle, 400+$x, 400+$y, $black, $font, 'ABCDEF');
  $s = vsprintf("(%d, %d), (%d, %d), (%d, %d), (%d, %d)\n", $bbox);
  echo $s;
  $temp = $cos_t * $x + $sin_t * $y;
  $y    = $cos_t * $y - $sin_t * $x;
  $x    = $temp;
}
imagepng($g, "$cwd/bug43073.png");
?>
<?php @unlink(dirname(__FILE__) . '/bug43073.png'); ?>