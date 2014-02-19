--TEST--
Different types of thumbnailing
--SKIPIF--
<?php require_once dirname(__FILE__) . '/skipif.inc'; ?>
--FILE--
<?php
echo "--- Source Image: 400x200, Imagick::thumbnailImage( 100, null, false )\n";
$imagick = new Imagick();
$imagick->newImage( 400, 200, "white" );
$imagick->thumbnailImage( 100, null, false );
$g = $imagick->getImageGeometry();
echo "{$g['width']}x{$g['height']}\n";

echo "--- Source Image: 400x200, Imagick::thumbnailImage( null, 100, false )\n";
$imagick = new Imagick();
$imagick->newImage( 400, 200, "white" );
$imagick->thumbnailImage( null, 100, false );
$g = $imagick->getImageGeometry();
echo "{$g['width']}x{$g['height']}\n";

echo "--- Source Image: 400x200, Imagick::thumbnailImage( 100, 100, false )\n";
$imagick = new Imagick();
$imagick->newImage( 400, 200, "white" );
$imagick->thumbnailImage( 100, 100, false);
$g = $imagick->getImageGeometry();
echo "{$g['width']}x{$g['height']}\n";

echo "--- Source Image: 400x200, Imagick::thumbnailImage( null, null, false )\n";
$imagick = new Imagick();
$imagick->newImage( 400, 200, "white" );
try
{
  $imagick->thumbnailImage( null, null, false );
  echo "FAILED TEST\n";
}
catch ( ImagickException $e )
{
  echo $e->getMessage() . "\n";
}

echo "--- Source Image: 400x200, Imagick::thumbnailImage( 100, 100, true )\n";
$imagick = new Imagick();
$imagick->newImage( 400, 200, "white" );
$imagick->thumbnailImage( 100, 100, true );
$g = $imagick->getImageGeometry();
echo "{$g['width']}x{$g['height']}\n";

echo "--- Source Image: 400x200, Imagick::thumbnailImage( 100, null, true )\n";
$imagick = new Imagick();
$imagick->newImage( 400, 200, "white" );
try
{
  $imagick->thumbnailImage( 100, null, true );
  echo "FAILED TEST\n";
}
catch ( ImagickException $e )
{
  echo $e->getMessage() . "\n";
}

echo "--- Source Image: 400x200, Imagick::thumbnailImage( null, 100, true )\n";
$imagick = new Imagick();
$imagick->newImage( 400, 200, "white" );
try
{
  $imagick->thumbnailImage( null, 100, true );
  echo "FAILED TEST\n";
}
catch ( ImagickException $e )
{
  echo $e->getMessage() . "\n";
}

echo "--- Source Image: 400x200, Imagick::thumbnailImage( null, null, true )\n";
$imagick = new Imagick();
$imagick->newImage( 400, 200, "white" );
try
{
  $imagick->thumbnailImage( null, null, true );
  echo "FAILED TEST\n";
}
catch ( ImagickException $e )
{
  echo $e->getMessage() . "\n";
}
?>
--EXPECTF--
--- Source Image: 400x200, Imagick::thumbnailImage( 100, null, false )
100x50
--- Source Image: 400x200, Imagick::thumbnailImage( null, 100, false )
200x100
--- Source Image: 400x200, Imagick::thumbnailImage( 100, 100, false )
100x100
--- Source Image: 400x200, Imagick::thumbnailImage( null, null, false )
Invalid image geometry
--- Source Image: 400x200, Imagick::thumbnailImage( 100, 100, true )
100x50
--- Source Image: 400x200, Imagick::thumbnailImage( 100, null, true )
Invalid image geometry
--- Source Image: 400x200, Imagick::thumbnailImage( null, 100, true )
Invalid image geometry
--- Source Image: 400x200, Imagick::thumbnailImage( null, null, true )
Invalid image geometry
