<?hh

<<__EntryPoint>>
function main() {
  $width = $height = 100;
  $chn = 0x1; // Corresponds to red channel
  $met = 0x1; // Corresponds to AbsoluteErrorMetric
  $dst = 0x1; // Corresponds to AffineDistortion
  $fn = 0x3; // Corresponds to ArcsinFunction
  $sparse = 0x1; // Corresponds to BarycentricColorInterpolate

  /* Create empty image */
  $im = new Imagick();
  $im->newImage($width, $height, 'gray');

  var_dump($im->compareImageChannels($im, $chn, $met));
  var_dump($im->compareImages($im, $met));
  $im->convolveImage(varray[0.1, 0.2, 0.3]);
  try {
    $im->distortImage($dst, varray[0.1], true);
  } catch (Exception $e) {
    printf("%s\n", $e->getMessage());
  }
  $im->functionImage($fn, varray[1.0]);
  $im->recolorImage(varray[1.0]);
  $im->setSamplingFactors(varray[1.0, 1.0, 1.0]);

  try {
    $im->sparseColorImage($sparse, varray[1.0, 2.0]);
  } catch (Exception $e) {
    printf("%s\n", $e->getMessage());
  }

  var_dump($im->getImageBluePrimary());
  var_dump($im->getImageChannelExtrema($chn));
  var_dump($im->getImageChannelKurtosis());
  var_dump($im->getImageChannelMean($chn));
  var_dump($im->getImageChannelRange($chn));
  var_dump($im->getImageChannelStatistics());
  var_dump($im->getImageExtrema());
  var_dump($im->getImageGreenPrimary());
  var_dump($im->getImageHistogram());
  var_dump($im->getImagePage());
  var_dump($im->getImageProfiles());
  var_dump($im->getImageProperties());
  var_dump($im->getImageRedPrimary());
  var_dump($im->getImageResolution());
  var_dump($im->getImageWhitePoint());
  var_dump($im->getPage());
  var_dump(Imagick::getQuantumDepth());
  var_dump(Imagick::getQuantumRange());
  var_dump($im->getSamplingFactors());
  var_dump($im->getSize());
  var_dump(Imagick::getVersion());
  var_dump($im->identifyImage());

  var_dump(is_varray(Imagick::queryFonts()));
  var_dump(Imagick::queryFormats());

}
