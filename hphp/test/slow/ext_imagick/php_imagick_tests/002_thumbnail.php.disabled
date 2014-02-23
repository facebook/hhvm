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
