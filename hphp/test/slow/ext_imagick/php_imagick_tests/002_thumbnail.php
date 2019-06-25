<?hh <<__EntryPoint>> function main(): void {
echo "--- Source Image: 400x200, Imagick::thumbnailImage( 100, 0, false )\n";
$imagick = new Imagick();
$imagick->newImage( 400, 200, "white" );
$imagick->thumbnailImage( 100, 0, false );
$g = $imagick->getImageGeometry();
echo "{$g['width']}x{$g['height']}\n";

echo "--- Source Image: 400x200, Imagick::thumbnailImage( 0, 100, false )\n";
$imagick = new Imagick();
$imagick->newImage( 400, 200, "white" );
$imagick->thumbnailImage( 0, 100, false );
$g = $imagick->getImageGeometry();
echo "{$g['width']}x{$g['height']}\n";

echo "--- Source Image: 400x200, Imagick::thumbnailImage( 100, 100, false )\n";
$imagick = new Imagick();
$imagick->newImage( 400, 200, "white" );
$imagick->thumbnailImage( 100, 100, false);
$g = $imagick->getImageGeometry();
echo "{$g['width']}x{$g['height']}\n";

echo "--- Source Image: 400x200, Imagick::thumbnailImage( 0, 0, false )\n";
$imagick = new Imagick();
$imagick->newImage( 400, 200, "white" );
try
{
  $imagick->thumbnailImage( 0, 0, false );
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

echo "--- Source Image: 400x200, Imagick::thumbnailImage( 100, 0, true )\n";
$imagick = new Imagick();
$imagick->newImage( 400, 200, "white" );
try
{
  $imagick->thumbnailImage( 100, 0, true );
  echo "FAILED TEST\n";
}
catch ( ImagickException $e )
{
  echo $e->getMessage() . "\n";
}

echo "--- Source Image: 400x200, Imagick::thumbnailImage( 0, 100, true )\n";
$imagick = new Imagick();
$imagick->newImage( 400, 200, "white" );
try
{
  $imagick->thumbnailImage( 0, 100, true );
  echo "FAILED TEST\n";
}
catch ( ImagickException $e )
{
  echo $e->getMessage() . "\n";
}

echo "--- Source Image: 400x200, Imagick::thumbnailImage( 0, 0, true )\n";
$imagick = new Imagick();
$imagick->newImage( 400, 200, "white" );
try
{
  $imagick->thumbnailImage( 0, 0, true );
  echo "FAILED TEST\n";
}
catch ( ImagickException $e )
{
  echo $e->getMessage() . "\n";
}
}
