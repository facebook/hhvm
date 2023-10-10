<?hh

class ImagickException extends RuntimeException { }
class ImagickDrawException extends RuntimeException { }
class ImagickPixelException extends RuntimeException { }
class ImagickPixelIteratorException extends RuntimeException { }

class Imagick implements Countable, Iterator {
  private ?resource $wand = null;
  private bool $nextOutOfBound = false;
  private bool $imagePending = false;

  <<__Native>>
  public function count(): int;

  <<__Native>>
  public function key(): int;

  <<__Native>>
  public function next(): void;

  <<__Native>>
  public function rewind(): void;

  /**
   * Adds adaptive blur filter to image
   *
   * @param float $radius - radius   The radius of the Gaussian, in
   *   pixels, not counting the center pixel. Provide a value of 0 and the
   *   radius will be chosen automagically.
   * @param float $sigma - sigma   The standard deviation of the
   *   Gaussian, in pixels.
   * @param int $channel - channel
   *
   * @return bool -
   */
  <<__Native>>
  public function adaptiveBlurImage(float $radius,
                             float $sigma,
                             int $channel = Imagick::CHANNEL_DEFAULT): bool;

  /**
   * Adaptively resize image with data dependent triangulation
   *
   * @param int $columns - columns   The number of columns in the scaled
   *   image.
   * @param int $rows - rows   The number of rows in the scaled image.
   * @param bool $bestfit - bestfit   Whether to fit the image inside a
   *   bounding box.
   *
   * @return bool -
   */
  <<__Native>>
  public function adaptiveResizeImage(int $columns,
                               int $rows,
                               bool $bestfit = false): bool;

  /**
   * Adaptively sharpen the image
   *
   * @param float $radius - radius   The radius of the Gaussian, in
   *   pixels, not counting the center pixel. Use 0 for auto-select.
   * @param float $sigma - sigma   The standard deviation of the
   *   Gaussian, in pixels.
   * @param int $channel - channel
   *
   * @return bool -
   */
  <<__Native>>
  public function adaptiveSharpenImage(float $radius,
                                float $sigma,
                                int $channel = Imagick::CHANNEL_DEFAULT): bool;

  /**
   * Selects a threshold for each pixel based on a range of intensity
   *
   * @param int $width - width   Width of the local neighborhood.
   * @param int $height - height   Height of the local neighborhood.
   * @param int $offset - offset   The mean offset
   *
   * @return bool -
   */
  <<__Native>>
  public function adaptiveThresholdImage(int $width,
                                  int $height,
                                  int $offset): bool;

  /**
   * Adds new image to Imagick object image list
   *
   * @param Imagick $source - source   The source Imagick object
   *
   * @return bool -
   */
  <<__Native>>
  public function addImage(Imagick $source): bool;

  /**
   * Adds random noise to the image
   *
   * @param int $noise_type - noise_type   The type of the noise. Refer
   *   to this list of noise constants.
   * @param int $channel - channel
   *
   * @return bool -
   */
  <<__Native>>
  public function addNoiseImage(int $noise_type,
                         int $channel = Imagick::CHANNEL_DEFAULT): bool;

  /**
   * Transforms an image
   *
   * @param ImagickDraw $matrix - matrix   The affine matrix
   *
   * @return bool -
   */
  <<__Native>>
  public function affineTransformImage(ImagickDraw $matrix): bool;

  /**
   * Animates an image or images
   *
   * @param string $x_server - x_server   X server address
   *
   * @return bool -
   */
  <<__Native>>
  public function animateImages(string $x_server): bool;

  /**
   * Annotates an image with text
   *
   * @param ImagickDraw $draw_settings - draw_settings   The ImagickDraw
   *   object that contains settings for drawing the text
   * @param float $x - x   Horizontal offset in pixels to the left of
   *   text
   * @param float $y - y   Vertical offset in pixels to the baseline of
   *   text
   * @param float $angle - angle   The angle at which to write the text
   * @param string $text - text   The string to draw
   *
   * @return bool -
   */
  <<__Native>>
  public function annotateImage(ImagickDraw $draw_settings,
                         float $x,
                         float $y,
                         float $angle,
                         string $text): bool;

  /**
   * Append a set of images
   *
   * @param bool $stack - stack   Whether to stack the images vertically.
   *   By default (or if FALSE is specified) images are stacked
   *   left-to-right. If stack is TRUE, images are stacked top-to-bottom.
   *
   * @return Imagick - Returns Imagick instance on success.
   */
  <<__Native>>
  public function appendImages(bool $stack = false): Imagick;

  /**
   * Average a set of images
   *
   * @return Imagick - Returns a new Imagick object on success.
   */
  <<__Native>>
  public function averageImages(): Imagick;

  /**
   * Forces all pixels below the threshold into black
   *
   * @param mixed $threshold - threshold   The threshold below which
   *   everything turns black
   *
   * @return bool -
   */
  <<__Native>>
  public function blackThresholdImage(mixed $threshold): bool;

  /**
   * Adds blur filter to image
   *
   * @param float $radius - radius   Blur radius
   * @param float $sigma - sigma   Standard deviation
   * @param int $channel - channel   The Channeltype constant. When not
   *   supplied, all channels are blurred.
   *
   * @return bool -
   */
  <<__Native>>
  public function blurImage(float $radius,
                     float $sigma,
                     int $channel = Imagick::CHANNEL_ALL): bool;

  /**
   * Surrounds the image with a border
   *
   * @param mixed $bordercolor - bordercolor   ImagickPixel object or a
   *   string containing the border color
   * @param int $width - width   Border width
   * @param int $height - height   Border height
   *
   * @return bool -
   */
  <<__Native>>
  public function borderImage(mixed $bordercolor,
                       int $width,
                       int $height): bool;

  /**
   * Simulates a charcoal drawing
   *
   * @param float $radius - radius   The radius of the Gaussian, in
   *   pixels, not counting the center pixel
   * @param float $sigma - sigma   The standard deviation of the
   *   Gaussian, in pixels   *
   * @return bool -
   */
  <<__Native>>
  public function charcoalImage(float $radius,
                         float $sigma): bool;

  /**
   * Removes a region of an image and trims
   *
   * @param int $width - width   Width of the chopped area
   * @param int $height - height   Height of the chopped area
   * @param int $x - x   X origo of the chopped area
   * @param int $y - y   Y origo of the chopped area
   *
   * @return bool -
   */
  <<__Native>>
  public function chopImage(int $width,
                     int $height,
                     int $x,
                     int $y): bool;

  /**
   * Clears all resources associated to Imagick object
   *
   * @return bool -
   */
  <<__Native>>
  public function clear(): bool;

  /**
   * Clips along the first path from the 8BIM profile
   *
   * @return bool -
   */
  <<__Native>>
  public function clipImage(): bool;

  /**
   * Clips along the named paths from the 8BIM profile
   *
   * @param string $pathname - pathname   The name of the path
   * @param bool $inside - inside   If TRUE later operations take effect
   *   inside clipping path. Otherwise later operations take effect outside
   *   clipping path.
   *
   * @return bool -
   */
  <<__Native>>
  public function clipPathImage(string $pathname,
                         bool $inside): bool;

  /**
   * Makes an exact copy of the Imagick object
   *
   * @return Imagick - A copy of the Imagick object is returned.
   */
  <<__Native>>
  public function __clone(): void;

  /**
   * Replaces colors in the image
   *
   * @param Imagick $lookup_table - lookup_table   Imagick object
   *   containing the color lookup table
   * @param int $channel - channel   The Channeltype constant. When not
   *   supplied, default channels are replaced.
   *
   * @return bool -
   */
  <<__Native>>
  public function clutImage(Imagick $lookup_table,
                     int $channel = Imagick::CHANNEL_DEFAULT): bool;

  /**
   * Composites a set of images
   *
   * @return Imagick - Returns a new Imagick object on success.
   */
  <<__Native>>
  public function coalesceImages(): Imagick;

  /**
   * Changes the color value of any pixel that matches target
   *
   * @param mixed $fill - fill   ImagickPixel object containing the fill
   *   color
   * @param float $fuzz - fuzz   The amount of fuzz. For example, set
   *   fuzz to 10 and the color red at intensities of 100 and 102
   *   respectively are now interpreted as the same color for the purposes
   *   of the floodfill.
   * @param mixed $bordercolor - bordercolor   ImagickPixel object
   *   containing the border color
   * @param int $x - x   X start position of the floodfill
   * @param int $y - y   Y start position of the floodfill
   *
   * @return bool -
   */
  <<__Native>>
  public function colorFloodfillImage(mixed $fill,
                               float $fuzz,
                               mixed $bordercolor,
                               int $x,
                               int $y): bool;

  /**
   * Blends the fill color with the image
   *
   * @param mixed $colorize - colorize   ImagickPixel object or a string
   *   containing the colorize color
   * @param mixed $opacity - opacity   ImagickPixel object or an float
   *   containing the opacity value. 1.0 is fully opaque and 0.0 is fully
   *   transparent.
   *
   * @return bool -
   */
  <<__Native>>
  public function colorizeImage(mixed $colorize,
                         mixed $opacity): bool;

  /**
   * Combines one or more images into a single image
   *
   * @param int $channelType - channelType   Provide any channel constant
   *   that is valid for your channel mode. To apply to more than one
   *   channel, combine channeltype constants using bitwise operators.
   *   Refer to this list of channel constants.
   *
   * @return Imagick -
   */
  <<__Native>>
  public function combineImages(int $channelType): Imagick;

  /**
   * Adds a comment to your image
   *
   * @param string $comment - comment   The comment to add
   *
   * @return bool -
   */
  <<__Native>>
  public function commentImage(string $comment): bool;

  /**
   * Returns the difference in one or more images
   *
   * @param Imagick $image - image   Imagick object containing the image
   *   to compare.
   * @param int $channelType - channelType   Provide any channel constant
   *   that is valid for your channel mode. To apply to more than one
   *   channel, combine channeltype constants using bitwise operators.
   *   Refer to this list of channel constants.
   * @param int $metricType - metricType   One of the metric type
   *   constants.
   *
   * @return array - Array consisting of new_wand and distortion.
   */
  <<__Native>>
  public function compareImageChannels(Imagick $image,
                                int $channelType,
                                int $metricType): varray<mixed>;

  /**
   * Returns the maximum bounding region between images
   *
   * @param int $method - method   One of the layer method constants.
   *
   * @return Imagick -
   */
  <<__Native>>
  public function compareImageLayers(int $method): Imagick;

  /**
   * Compares an image to a reconstructed image
   *
   * @param Imagick $compare - compare   An image to compare to.
   * @param int $metric - metric   Provide a valid metric type constant.
   *   Refer to this list of metric constants.
   *
   * @return array -
   */
  <<__Native>>
  public function compareImages(Imagick $compare,
                         int $metric): varray<mixed>;

  /**
   * Composite one image onto another
   *
   * @param Imagick $composite_object - composite_object   Imagick object
   *   which holds the composite image
   * @param int $composite -
   * @param int $x - x   The column offset of the composited image
   * @param int $y - y   The row offset of the composited image
   * @param int $channel - channel   Provide any channel constant that is
   *   valid for your channel mode. To apply to more than one channel,
   *   combine channeltype constants using bitwise operators. Refer to this
   *   list of channel constants.
   *
   * @return bool -
   */
  <<__Native>>
  public function compositeImage(Imagick $composite_object,
                          int $composite,
                          int $x,
                          int $y,
                          int $channel = Imagick::CHANNEL_ALL): bool;

  /**
   * The Imagick constructor
   *
   * @param mixed $files -
   *
   * @return  - Returns a new Imagick object on success.
   */
  <<__Native>>
  public function __construct(mixed $files = null): void;

  /**
   * Change the contrast of the image
   *
   * @param bool $sharpen - sharpen   The sharpen value
   *
   * @return bool -
   */
  <<__Native>>
  public function contrastImage(bool $sharpen): bool;

  /**
   * Enhances the contrast of a color image
   *
   * @param float $black_point - black_point   The black point.
   * @param float $white_point - white_point   The white point.
   * @param int $channel - channel   Provide any channel constant that is
   *   valid for your channel mode. To apply to more than one channel,
   *   combine channeltype constants using bitwise operators.
   *   Imagick::CHANNEL_ALL. Refer to this list of channel constants.
   *
   * @return bool -
   */
  <<__Native>>
  public function contrastStretchImage(float $black_point,
                                float $white_point,
                                int $channel = Imagick::CHANNEL_ALL): bool;

  /**
   * Applies a custom convolution kernel to the image
   *
   * @param array $kernel - kernel   The convolution kernel
   * @param int $channel - channel   Provide any channel constant that is
   *   valid for your channel mode. To apply to more than one channel,
   *   combine channeltype constants using bitwise operators. Refer to this
   *   list of channel constants.
   *
   * @return bool -
   */
  <<__Native>>
  public function convolveImage(varray<float> $kernel,
                         int $channel = Imagick::CHANNEL_ALL): bool;

  /**
   * Extracts a region of the image
   *
   * @param int $width - width   The width of the crop
   * @param int $height - height   The height of the crop
   * @param int $x - x   The X coordinate of the cropped region's top
   *   left corner
   * @param int $y - y   The Y coordinate of the cropped region's top
   *   left corner
   *
   * @return bool -
   */
  <<__Native>>
  public function cropImage(int $width,
                     int $height,
                     int $x,
                     int $y): bool;

  /**
   * Creates a crop thumbnail
   *
   * @param int $width - width   The width of the thumbnail
   * @param int $height - height   The Height of the thumbnail
   *
   * @return bool -
   */
  <<__Native>>
  public function cropThumbnailImage(int $width,
                              int $height): bool;

  /**
   * Returns a reference to the current Imagick object
   *
   * @return Imagick - Returns self on success.
   */
  <<__Native>>
  public function current(): Imagick;

  /**
   * Displaces an image's colormap
   *
   * @param int $displace - displace   The amount to displace the
   *   colormap.
   *
   * @return bool -
   */
  <<__Native>>
  public function cycleColormapImage(int $displace): bool;

  /**
   * Deciphers an image
   *
   * @param string $passphrase - passphrase   The passphrase
   *
   * @return bool -
   */
  <<__Native>>
  public function decipherImage(string $passphrase): bool;

  /**
   * Returns certain pixel differences between images
   *
   * @return Imagick - Returns a new Imagick object on success.
   */
  <<__Native>>
  public function deconstructImages(): Imagick;

  /**
   * Delete image artifact
   *
   * @param string $artifact - artifact   The name of the artifact to
   *   delete
   *
   * @return bool -
   */
  <<__Native>>
  public function deleteImageArtifact(string $artifact): bool;

  /**
   * Removes skew from the image
   *
   * @param float $threshold - threshold   Deskew threshold
   *
   * @return bool -
   */
  <<__Native>>
  public function deskewImage(float $threshold): bool;

  /**
   * Reduces the speckle noise in an image
   *
   * @return bool -
   */
  <<__Native>>
  public function despeckleImage(): bool;

  /**
   * Destroys the Imagick object
   *
   * @return bool -
   */
  <<__Native>>
  public function destroy(): bool;

  /**
   * Displays an image
   *
   * @param string $servername - servername   The X server name
   *
   * @return bool -
   */
  <<__Native>>
  public function displayImage(string $servername): bool;

  /**
   * Displays an image or image sequence
   *
   * @param string $servername - servername   The X server name
   *
   * @return bool -
   */
  <<__Native>>
  public function displayImages(string $servername): bool;

  /**
   * Distorts an image using various distortion methods
   *
   * @param int $method - method   The method of image distortion. See
   *   distortion constants
   * @param array $arguments - arguments   The arguments for this
   *   distortion method
   * @param bool $bestfit - bestfit   Attempt to resize destination to
   *   fit distorted source
   *
   * @return bool -
   */
  <<__Native>>
  public function distortImage(int $method,
                        varray<float> $arguments,
                        bool $bestfit): bool;

  /**
   * Renders the ImagickDraw object on the current image
   *
   * @param ImagickDraw $draw - draw   The drawing operations to render
   *   on the image.
   *
   * @return bool -
   */
  <<__Native>>
  public function drawImage(ImagickDraw $draw): bool;

  /**
   * Enhance edges within the image
   *
   * @param float $radius - radius   The radius of the operation.
   *
   * @return bool -
   */
  <<__Native>>
  public function edgeImage(float $radius): bool;

  /**
   * Returns a grayscale image with a three-dimensional effect
   *
   * @param float $radius - radius   The radius of the effect
   * @param float $sigma - sigma   The sigma of the effect
   *
   * @return bool -
   */
  <<__Native>>
  public function embossImage(float $radius,
                       float $sigma): bool;

  /**
   * Enciphers an image
   *
   * @param string $passphrase - passphrase   The passphrase
   *
   * @return bool -
   */
  <<__Native>>
  public function encipherImage(string $passphrase): bool;

  /**
   * Improves the quality of a noisy image
   *
   * @return bool -
   */
  <<__Native>>
  public function enhanceImage(): bool;

  /**
   * Equalizes the image histogram
   *
   * @return bool -
   */
  <<__Native>>
  public function equalizeImage(): bool;

  /**
   * Applies an expression to an image
   *
   * @param int $op - op   The evaluation operator
   * @param float $constant - constant   The value of the operator
   * @param int $channel - channel   Provide any channel constant that is
   *   valid for your channel mode. To apply to more than one channel,
   *   combine channeltype constants using bitwise operators. Refer to this
   *   list of channel constants.
   *
   * @return bool -
   */
  <<__Native>>
  public function evaluateImage(int $op,
                         float $constant,
                         int $channel = Imagick::CHANNEL_ALL): bool;

  /**
   * Exports raw image pixels
   *
   * @param int $x - x   X-coordinate of the exported area
   * @param int $y - y   Y-coordinate of the exported area
   * @param int $width - width   Width of the exported aread
   * @param int $height - height   Height of the exported area
   * @param string $map - map   Ordering of the exported pixels. For
   *   example "RGB". Valid characters for the map are R, G, B, A, O, C, Y,
   *   M, K, I and P.
   * @param int $storage - storage   Refer to this list of pixel type
   *   constants
   *
   * @return array - Returns an array containing the pixels values.
   */
  <<__Native>>
  public function exportImagePixels(int $x,
                                    int $y,
                                    int $width,
                                    int $height,
                                    string $map,
                                    int $storage): varray<int>;

  /**
   * Set image size
   *
   * @param int $width - width   The new width
   * @param int $height - height   The new height
   * @param int $x - x   X position for the new size
   * @param int $y - y   Y position for the new size
   *
   * @return bool -
   */
  <<__Native>>
  public function extentImage(int $width,
                       int $height,
                       int $x,
                       int $y): bool;

  /**
   * Merges a sequence of images
   *
   * @return Imagick -
   */
  <<__Native>>
  public function flattenImages(): Imagick;

  /**
   * Creates a vertical mirror image
   *
   * @return bool -
   */
  <<__Native>>
  public function flipImage(): bool;

  /**
   * Changes the color value of any pixel that matches target
   *
   * @param mixed $fill - fill   ImagickPixel object or a string
   *   containing the fill color
   * @param float $fuzz - fuzz
   * @param mixed $target - target   ImagickPixel object or a string
   *   containing the target color to paint
   * @param int $x - x   X start position of the floodfill
   * @param int $y - y   Y start position of the floodfill
   * @param bool $invert - invert   If TRUE paints any pixel that does
   *   not match the target color.
   * @param int $channel - channel
   *
   * @return bool -
   */
  <<__Native>>
  public function floodFillPaintImage(mixed $fill,
                               float $fuzz,
                               mixed $target,
                               int $x,
                               int $y,
                               bool $invert,
                               int $channel = Imagick::CHANNEL_DEFAULT): bool;

  /**
   * Creates a horizontal mirror image
   *
   * @return bool -
   */
  <<__Native>>
  public function flopImage(): bool;

  /**
   * Adds a simulated three-dimensional border
   *
   * @param mixed $matte_color - matte_color   ImagickPixel object or a
   *   string representing the matte color
   * @param int $width - width   The width of the border
   * @param int $height - height   The height of the border
   * @param int $inner_bevel - inner_bevel   The inner bevel width
   * @param int $outer_bevel - outer_bevel   The outer bevel width
   *
   * @return bool -
   */
  <<__Native>>
  public function frameImage(mixed $matte_color,
                      int $width,
                      int $height,
                      int $inner_bevel,
                      int $outer_bevel): bool;

  /**
   * Applies a function on the image
   *
   * @param int $function - function   Refer to this list of function
   *   constants
   * @param array $arguments - arguments   Array of arguments to pass to
   *   this function.
   * @param int $channel -
   *
   * @return bool -
   */
  <<__Native>>
  public function functionImage(int $function,
                                varray<float> $arguments,
                                int $channel = Imagick::CHANNEL_DEFAULT): bool;

  /**
   * Evaluate expression for each pixel in the image
   *
   * @param string $expression - expression   The expression.
   * @param int $channel - channel   Provide any channel constant that is
   *   valid for your channel mode. To apply to more than one channel,
   *   combine channeltype constants using bitwise operators. Refer to this
   *   list of channel constants.
   *
   * @return Imagick -
   */
  <<__Native>>
  public function fxImage(string $expression,
                   int $channel = Imagick::CHANNEL_ALL): Imagick;

  /**
   * Gamma-corrects an image
   *
   * @param float $gamma - gamma   The amount of gamma-correction.
   * @param int $channel - channel   Provide any channel constant that is
   *   valid for your channel mode. To apply to more than one channel,
   *   combine channeltype constants using bitwise operators. Refer to this
   *   list of channel constants.
   *
   * @return bool -
   */
  <<__Native>>
  public function gammaImage(float $gamma,
                      int $channel = Imagick::CHANNEL_ALL): bool;

  /**
   * Blurs an image
   *
   * @param float $radius - radius   The radius of the Gaussian, in
   *   pixels, not counting the center pixel.
   * @param float $sigma - sigma   The standard deviation of the
   *   Gaussian, in pixels.
   * @param int $channel - channel   Provide any channel constant that is
   *   valid for your channel mode. To apply to more than one channel,
   *   combine channeltype constants using bitwise operators. Refer to this
   *   list of channel constants.
   *
   * @return bool -
   */
  <<__Native>>
  public function gaussianBlurImage(float $radius,
                             float $sigma,
                             int $channel = Imagick::CHANNEL_ALL): bool;

  /**
   * Gets the colorspace
   *
   * @return int - Returns an integer which can be compared against
   *   COLORSPACE constants.
   */
  <<__Native>>
  public function getColorspace(): int;

  /**
   * Gets the object compression type
   *
   * @return int - Returns the compression constant
   */
  <<__Native>>
  public function getCompression(): int;

  /**
   * Gets the current image's compression quality
   *
   * @return int - Returns integer describing the images compression
   *   quality
   */
  <<__Native>>
  public function getCompressionQuality(): int;

  /**
   * Returns the ImageMagick API copyright as a string
   *
   * @return string - Returns a string containing the copyright notice of
   *   Imagemagick and Magickwand C API.
   */
  <<__Native>>
  public static function getCopyright(): string;

  /**
   * The filename associated with an image sequence
   *
   * @return string - Returns a string on success.
   */
  <<__Native>>
  public function getFilename(): string;

  /**
   * Gets font
   *
   * @return string - Returns the string containing the font name or
   *   FALSE if not font is set.
   */
  <<__Native>>
  public function getFont(): string;

  /**
   * Returns the format of the Imagick object
   *
   * @return string - Returns the format of the image.
   */
  <<__Native>>
  public function getFormat(): string;

  /**
   * Gets the gravity
   *
   * @return int - Returns the gravity property. Refer to the list of
   *   gravity constants.
   */
  <<__Native>>
  public function getGravity(): int;

  /**
   * Returns the ImageMagick home URL
   *
   * @return string - Returns a link to the imagemagick homepage.
   */
  <<__Native>>
  public static function getHomeURL(): string;

  /**
   * Returns a new Imagick object
   *
   * @return Imagick - Returns a new Imagick object with the current
   *   image sequence.
   */
  <<__Native>>
  public function getImage(): Imagick;

  /**
   * Gets the image alpha channel
   *
   * @return int - Returns a constant defining the current alpha channel
   *   value. Refer to this list of alpha channel constants.
   */
  <<__Native>>
  public function getImageAlphaChannel(): int;

  /**
   * Get image artifact
   *
   * @param string $artifact - artifact   The name of the artifact
   *
   * @return string - Returns the artifact value on success.
   */
  <<__Native>>
  public function getImageArtifact(string $artifact): string;

  /**
   * Returns the image background color
   *
   * @return ImagickPixel - Returns an ImagickPixel set to the background
   *   color of the image.
   */
  <<__Native>>
  public function getImageBackgroundColor(): ImagickPixel;

  /**
   * Returns the image sequence as a blob
   *
   * @return string - Returns a string containing the image.
   */
  <<__Native>>
  public function getImageBlob(): string;

  /**
   * Returns the chromaticy blue primary point
   *
   * @return array - Array consisting of "x" and "y" coordinates of
   *   point.
   */
  <<__Native>>
  public function getImageBluePrimary(): darray<string, mixed>;

  /**
   * Returns the image border color
   *
   * @return ImagickPixel -
   */
  <<__Native>>
  public function getImageBorderColor(): ImagickPixel;

  /**
   * Gets the depth for a particular image channel
   *
   * @param int $channel - channel
   *
   * @return int -
   */
  <<__Native>>
  public function getImageChannelDepth(int $channel): int;

  /**
   * Compares image channels of an image to a reconstructed image
   *
   * @param Imagick $reference - reference   Imagick object to compare
   *   to.
   * @param int $channel - channel   Provide any channel constant that is
   *   valid for your channel mode. To apply to more than one channel,
   *   combine channeltype constants using bitwise operators. Refer to this
   *   list of channel constants.
   * @param int $metric - metric   One of the metric type constants.
   *
   * @return float -
   */
  <<__Native>>
  public function getImageChannelDistortion(Imagick $reference,
                                     int $channel,
                                     int $metric): float;

  /**
   * Gets channel distortions
   *
   * @param Imagick $reference - reference   Imagick object containing
   *   the reference image
   * @param int $metric - metric   Refer to this list of metric type
   *   constants.
   * @param int $channel - channel
   *
   * @return float - Returns a double describing the channel distortion.
   */
  <<__Native>>
  public function getImageChannelDistortions(Imagick $reference,
                                      int $metric,
                                      int $channel = Imagick::CHANNEL_DEFAULT): float;

  /**
   * Gets the extrema for one or more image channels
   *
   * @param int $channel - channel   Provide any channel constant that is
   *   valid for your channel mode. To apply to more than one channel,
   *   combine channeltype constants using bitwise operators. Refer to this
   *   list of channel constants.
   *
   * @return array -
   */
  <<__Native>>
  public function getImageChannelExtrema(int $channel): darray<string, int>;

  /**
   * The getImageChannelKurtosis purpose
   *
   * @param int $channel - channel
   *
   * @return array - Returns an array with kurtosis and skewness members.
   */
  <<__Native>>
  public function getImageChannelKurtosis(
    int $channel = Imagick::CHANNEL_DEFAULT,
  ): darray<string, float>;

  /**
   * Gets the mean and standard deviation
   *
   * @param int $channel - channel   Provide any channel constant that is
   *   valid for your channel mode. To apply to more than one channel,
   *   combine channeltype constants using bitwise operators. Refer to this
   *   list of channel constants.
   *
   * @return array -
   */
  <<__Native>>
  public function getImageChannelMean(int $channel): darray<string, float>;

  /**
   * Gets channel range
   *
   * @param int $channel - channel
   *
   * @return array - Returns an array containing minima and maxima values
   *   of the channel(s).
   */
  <<__Native>>
  public function getImageChannelRange(int $channel): darray<string, float>;

  /**
   * Returns statistics for each channel in the image
   *
   * @return array -
   */
  <<__Native>>
  public function getImageChannelStatistics(): darray<int, darray<string, num>>;

  /**
   * Gets image clip mask
   *
   * @return Imagick - Returns an Imagick object containing the clip
   *   mask.
   */
  <<__Native>>
  public function getImageClipMask(): Imagick;

  /**
   * Returns the color of the specified colormap index
   *
   * @param int $index - index   The offset into the image colormap.
   *
   * @return ImagickPixel -
   */
  <<__Native>>
  public function getImageColormapColor(int $index): ImagickPixel;

  /**
   * Gets the number of unique colors in the image
   *
   * @return int -
   */
  <<__Native>>
  public function getImageColors(): int;

  /**
   * Gets the image colorspace
   *
   * @return int -
   */
  <<__Native>>
  public function getImageColorspace(): int;

  /**
   * Returns the composite operator associated with the image
   *
   * @return int -
   */
  <<__Native>>
  public function getImageCompose(): int;

  /**
   * Gets the current image's compression type
   *
   * @return int - Returns the compression constant
   */
  <<__Native>>
  public function getImageCompression(): int;

  /**
   * Gets the image delay
   *
   * @return int - Returns the image delay.
   */
  <<__Native>>
  public function getImageDelay(): int;

  /**
   * Gets the image depth
   *
   * @return int - The image depth.
   */
  <<__Native>>
  public function getImageDepth(): int;

  /**
   * Gets the image disposal method
   *
   * @return int - Returns the dispose method on success.
   */
  <<__Native>>
  public function getImageDispose(): int;

  /**
   * Compares an image to a reconstructed image
   *
   * @param magickwand $reference - reference   Imagick object to compare
   *   to.
   * @param int $metric - metric   One of the metric type constants.
   *
   * @return float - Returns the distortion metric used on the image (or
   *   the best guess thereof).
   */
  <<__Native>>
  public function getImageDistortion(magickwand $reference,
                              int $metric): float;

  /**
   * Gets the extrema for the image
   *
   * @return array - Returns an associative array with the keys "min" and
   *   "max".
   */
  <<__Native>>
  public function getImageExtrema(): darray<string, int>;

  /**
   * Returns the filename of a particular image in a sequence
   *
   * @return string - Returns a string with the filename of the image.
   */
  <<__Native>>
  public function getImageFilename(): string;

  /**
   * Returns the format of a particular image in a sequence
   *
   * @return string - Returns a string containing the image format on
   *   success.
   */
  <<__Native>>
  public function getImageFormat(): string;

  /**
   * Gets the image gamma
   *
   * @return float - Returns the image gamma on success.
   */
  <<__Native>>
  public function getImageGamma(): float;

  /**
   * Gets the width and height as an associative array
   *
   * @return array - Returns an array with the width/height of the image.
   */
  <<__Native>>
  public function getImageGeometry(): darray<string, int>;

  /**
   * Gets the image gravity
   *
   * @return int - Returns the images gravity property. Refer to the list
   *   of gravity constants.
   */
  <<__Native>>
  public function getImageGravity(): int;

  /**
   * Returns the chromaticy green primary point
   *
   * @return array - Returns an array with the keys "x" and "y" on
   *   success, throws an ImagickException on failure.
   */
  <<__Native>>
  public function getImageGreenPrimary(): darray<string, float>;

  /**
   * Returns the image height
   *
   * @return int - Returns the image height in pixels.
   */
  <<__Native>>
  public function getImageHeight(): int;

  /**
   * Gets the image histogram
   *
   * @return array - Returns the image histogram as an array of
   *   ImagickPixel objects.
   */
  <<__Native>>
  public function getImageHistogram(): varray<ImagickPixel>;

  /**
   * Gets the index of the current active image
   *
   * @return int - Returns an integer containing the index of the image
   *   in the stack.
   */
  <<__Native>>
  public function getImageIndex(): int;

  /**
   * Gets the image interlace scheme
   *
   * @return int - Returns the interlace scheme as an integer on success.
   */
  <<__Native>>
  public function getImageInterlaceScheme(): int;

  /**
   * Returns the interpolation method
   *
   * @return int - Returns the interpolate method on success.
   */
  <<__Native>>
  public function getImageInterpolateMethod(): int;

  /**
   * Gets the image iterations
   *
   * @return int - Returns the image iterations as an integer.
   */
  <<__Native>>
  public function getImageIterations(): int;

  /**
   * Returns the image length in bytes
   *
   * @return int - Returns an int containing the current image size.
   */
  <<__Native>>
  public function getImageLength(): int;

  /**
   * Return if the image has a matte channel
   *
   * @return bool -
   */
  <<__Native>>
  public function getImageMatte(): bool;

  /**
   * Returns the image matte color
   *
   * @return ImagickPixel - Returns ImagickPixel object on success.
   */
  <<__Native>>
  public function getImageMatteColor(): ImagickPixel;

  <<__Native>>
  public function getImageMimeType(): string;

  /**
   * Gets the image orientation
   *
   * @return int - Returns an int on success.
   */
  <<__Native>>
  public function getImageOrientation(): int;

  /**
   * Returns the page geometry
   *
   * @return array - Returns the page geometry associated with the image
   *   in an array with the keys "width", "height", "x", and "y".
   */
  <<__Native>>
  public function getImagePage(): darray<string, int>;

  /**
   * Returns the color of the specified pixel
   *
   * @param int $x - x   The x-coordinate of the pixel
   * @param int $y - y   The y-coordinate of the pixel
   *
   * @return ImagickPixel - Returns an ImagickPixel instance for the
   *   color at the coordinates given.
   */
  <<__Native>>
  public function getImagePixelColor(int $x,
                              int $y): ImagickPixel;

  /**
   * Returns the named image profile
   *
   * @param string $name - name   The name of the profile to return.
   *
   * @return string - Returns a string containing the image profile.
   */
  <<__Native>>
  public function getImageProfile(string $name): string;

  /**
   * Returns the image profiles
   *
   * @param string $pattern - pattern   The pattern for profile names.
   * @param bool $only_names - only_names   Whether to return only
   *   profile names. If FALSE then values are returned as well
   *
   * @return array - Returns an array containing the image profiles or
   *   profile names.
   */
  <<__Native>>
  public function getImageProfiles(string $pattern = '*',
                            bool $with_values = true): varray_or_darray<mixed>;

  /**
   * Returns the image properties
   *
   * @param string $pattern - pattern   The pattern for property names.
   * @param bool $only_names - only_names   Whether to return only
   *   property names. If FALSE then also the values are returned
   *
   * @return array - Returns an array containing the image properties or
   *   property names.
   */
  <<__Native>>
  public function getImageProperties(
    string $pattern = '*',
    bool $with_values = true,
  ): varray_or_darray<mixed>;

  /**
   * Returns the named image property
   *
   * @param string $name - name   name of the property (for example
   *   Exif:DateTime)
   *
   * @return string - Returns a string containing the image property,
   *   false if a property with the given name does not exist.
   */
  <<__Native>>
  public function getImageProperty(string $name): string;

  /**
   * Returns the chromaticity red primary point
   *
   * @return array - Returns the chromaticity red primary point as an
   *   array with the keys "x" and "y".
   */
  <<__Native>>
  public function getImageRedPrimary(): darray<string, float>;

  /**
   * Extracts a region of the image
   *
   * @param int $width - width   The width of the extracted region.
   * @param int $height - height   The height of the extracted region.
   * @param int $x - x   X-coordinate of the top-left corner of the
   *   extracted region.
   * @param int $y - y   Y-coordinate of the top-left corner of the
   *   extracted region.
   *
   * @return Imagick - Extracts a region of the image and returns it as a
   *   new wand.
   */
  <<__Native>>
  public function getImageRegion(int $width,
                          int $height,
                          int $x,
                          int $y): Imagick;

  /**
   * Gets the image rendering intent
   *
   * @return int - Returns the image rendering intent.
   */
  <<__Native>>
  public function getImageRenderingIntent(): int;

  /**
   * Gets the image X and Y resolution
   *
   * @return array - Returns the resolution as an array.
   */
  <<__Native>>
  public function getImageResolution(): darray<string, float>;

  /**
   * Returns all image sequences as a blob
   *
   * @return string - Returns a string containing the images. On failure,
   *   throws ImagickException.
   */
  <<__Native>>
  public function getImagesBlob(): string;

  /**
   * Gets the image scene
   *
   * @return int - Returns the image scene.
   */
  <<__Native>>
  public function getImageScene(): int;

  /**
   * Generates an SHA-256 message digest
   *
   * @return string - Returns a string containing the SHA-256 hash of the
   *   file.
   */
  <<__Native>>
  public function getImageSignature(): string;

  /**
   * Returns the image length in bytes
   *
   * @return int - Returns an int containing the current image size.
   */
  <<__Native>>
  public function getImageSize(): int;

  /**
   * Gets the image ticks-per-second
   *
   * @return int - Returns the image ticks-per-second.
   */
  <<__Native>>
  public function getImageTicksPerSecond(): int;

  /**
   * Gets the image total ink density
   *
   * @return float - Returns the image total ink density of the image.
   */
  <<__Native>>
  public function getImageTotalInkDensity(): float;

  /**
   * Gets the potential image type
   *
   * @return int - Returns the potential image type.
   *   Imagick::IMGTYPE_UNDEFINED     Imagick::IMGTYPE_BILEVEL
   *   Imagick::IMGTYPE_GRAYSCALE     Imagick::IMGTYPE_GRAYSCALEMATTE
   *   Imagick::IMGTYPE_PALETTE     Imagick::IMGTYPE_PALETTEMATTE
   *   Imagick::IMGTYPE_TRUECOLOR     Imagick::IMGTYPE_TRUECOLORMATTE
   *   Imagick::IMGTYPE_COLORSEPARATION
   *   Imagick::IMGTYPE_COLORSEPARATIONMATTE     Imagick::IMGTYPE_OPTIMIZE
   */
  <<__Native>>
  public function getImageType(): int;

  /**
   * Gets the image units of resolution
   *
   * @return int - Returns the image units of resolution.
   */
  <<__Native>>
  public function getImageUnits(): int;

  /**
   * Returns the virtual pixel method
   *
   * @return int - Returns the virtual pixel method on success.
   */
  <<__Native>>
  public function getImageVirtualPixelMethod(): int;

  /**
   * Returns the chromaticity white point
   *
   * @return array - Returns the chromaticity white point as an
   *   associative array with the keys "x" and "y".
   */
  <<__Native>>
  public function getImageWhitePoint(): darray<string, float>;

  /**
   * Returns the image width
   *
   * @return int - Returns the image width.
   */
  <<__Native>>
  public function getImageWidth(): int;

  /**
   * Gets the object interlace scheme
   *
   * @return int - Gets the wand interlace scheme.
   */
  <<__Native>>
  public function getInterlaceScheme(): int;

  /**
   * Gets the index of the current active image
   *
   * @return int - Returns an integer containing the index of the image
   *   in the stack.
   */
  <<__Native>>
  public function getIteratorIndex(): int;

  /**
   * Returns the number of images in the object
   *
   * @return int - Returns the number of images associated with Imagick
   *   object.
   */
  <<__Native>>
  public function getNumberImages(): int;

  /**
   * Returns a value associated with the specified key
   *
   * @param string $key - key   The name of the option
   *
   * @return string - Returns a value associated with a wand and the
   *   specified key.
   */
  <<__Native>>
  public function getOption(string $key): string;

  /**
   * Returns the ImageMagick package name
   *
   * @return string - Returns the ImageMagick package name as a string.
   */
  <<__Native>>
  public static function getPackageName(): string;

  /**
   * Returns the page geometry
   *
   * @return array - Returns the page geometry associated with the
   *   Imagick object in an associative array with the keys "width",
   *   "height", "x", and "y", throwing ImagickException on error.
   */
  <<__Native>>
  public function getPage(): darray<string, int>;

  /**
   * Returns a MagickPixelIterator
   *
   * @return ImagickPixelIterator - Returns an ImagickPixelIterator on
   *   success.
   */
  <<__Native>>
  public function getPixelIterator(): ImagickPixelIterator;

  /**
   * Get an ImagickPixelIterator for an image section
   *
   * @param int $x - x   The x-coordinate of the region.
   * @param int $y - y   The y-coordinate of the region.
   * @param int $columns - columns   The width of the region.
   * @param int $rows - rows   The height of the region.
   *
   * @return ImagickPixelIterator - Returns an ImagickPixelIterator for
   *   an image section.
   */
  <<__Native>>
  public function getPixelRegionIterator(int $x,
                                  int $y,
                                  int $columns,
                                  int $rows): ImagickPixelIterator;

  /**
   * Gets point size
   *
   * @return float - Returns a containing the point size.
   */
  <<__Native>>
  public function getPointSize(): float;

  /**
   * Gets the quantum depth
   *
   * @return array - Returns the Imagick quantum depth as a string.
   */
  <<__Native>>
  public static function getQuantumDepth(): darray<string, mixed>;

  /**
   * Returns the Imagick quantum range
   *
   * @return array - Returns the Imagick quantum range as a string.
   */
  <<__Native>>
  public static function getQuantumRange(): darray<string, mixed>;

  /**
   * Returns the ImageMagick release date
   *
   * @return string - Returns the ImageMagick release date as a string.
   */
  <<__Native>>
  public static function getReleaseDate(): string;

  /**
   * Returns the specified resource's memory usage
   *
   * @param int $type - type   Refer to the list of resourcetype
   *   constants.
   *
   * @return int - Returns the specified resource's memory usage in
   *   megabytes.
   */
  <<__Native>>
  public static function getResource(int $type): int;

  /**
   * Returns the specified resource limit
   *
   * @param int $type - type   Refer to the list of resourcetype
   *   constants.
   *
   * @return int - Returns the specified resource limit in megabytes.
   */
  <<__Native>>
  public static function getResourceLimit(int $type): int;

  /**
   * Gets the horizontal and vertical sampling factor
   *
   * @return array - Returns an associative array with the horizontal and
   *   vertical sampling factors of the image.
   */
  <<__Native>>
  public function getSamplingFactors(): varray<float>;

  /**
   * Returns the size associated with the Imagick object
   *
   * @return array - Returns the size associated with the Imagick object
   *   as an array with the keys "columns" and "rows".
   */
  <<__Native>>
  public function getSize(): darray<string, int>;

  /**
   * Returns the size offset
   *
   * @return int - Returns the size offset associated with the Imagick
   *   object.
   */
  <<__Native>>
  public function getSizeOffset(): int;

  /**
   * Returns the ImageMagick API version
   *
   * @return array - Returns the ImageMagick API version as a string and
   *   as a number.
   */
  <<__Native>>
  public static function getVersion(): darray<string, mixed>;

  /**
   * Replaces colors in the image
   *
   * @param Imagick $clut - clut   Imagick object containing the Hald
   *   lookup image.
   * @param int $channel - channel
   *
   * @return bool -
   */
  <<__Native>>
  public function haldClutImage(Imagick $clut,
                                int $channel = Imagick::CHANNEL_DEFAULT): bool;

  /**
   * Checks if the object has more images
   *
   * @return bool - Returns TRUE if the object has more images when
   *   traversing the list in the forward direction, returns FALSE if there
   *   are none.
   */
  <<__Native>>
  public function hasNextImage(): bool;

  /**
   * Checks if the object has a previous image
   *
   * @return bool - Returns TRUE if the object has more images when
   *   traversing the list in the reverse direction, returns FALSE if there
   *   are none.
   */
  <<__Native>>
  public function hasPreviousImage(): bool;

  /**
   * Identifies an image and fetches attributes
   *
   * @param bool $appendRawOutput - appendRawOutput
   *
   * @return array - Identifies an image and returns the attributes.
   *   Attributes include the image width, height, size, and others.
   */
  <<__Native>>
  public function identifyImage(
    bool $appendRawOutput = false,
  ): darray<arraykey, mixed>;

  /**
   * Creates a new image as a copy
   *
   * @param float $radius - radius   The radius of the implode
   *
   * @return bool -
   */
  <<__Native>>
  public function implodeImage(float $radius): bool;

  /**
   * Imports image pixels
   *
   * @param int $x - x   The image x position
   * @param int $y - y   The image y position
   * @param int $width - width   The image width
   * @param int $height - height   The image height
   * @param string $map - map   Map of pixel ordering as a string. This
   *   can be for example RGB. The value can be any combination or order of
   *   R = red, G = green, B = blue, A = alpha (0 is transparent), O =
   *   opacity (0 is opaque), C = cyan, Y = yellow, M = magenta, K = black,
   *   I = intensity (for grayscale), P = pad.
   * @param int $storage - storage   The pixel storage method. Refer to
   *   this list of pixel constants.
   * @param array $pixels - pixels   The array of pixels
   *
   * @return bool -
   */
  <<__Native>>
  public function importImagePixels(int $x,
                                    int $y,
                                    int $width,
                                    int $height,
                                    string $map,
                                    int $storage,
                                    varray<float> $pixels): bool;

  /**
   * Adds a label to an image
   *
   * @param string $label - label   The label to add
   *
   * @return bool -
   */
  <<__Native>>
  public function labelImage(string $label): bool;

  /**
   * Adjusts the levels of an image
   *
   * @param float $blackPoint - blackPoint   The image black point
   * @param float $gamma - gamma   The gamma value
   * @param float $whitePoint - whitePoint   The image white point
   * @param int $channel - channel   Provide any channel constant that is
   *   valid for your channel mode. To apply to more than one channel,
   *   combine channeltype constants using bitwise operators. Refer to this
   *   list of channel constants.
   *
   * @return bool -
   */
  <<__Native>>
  public function levelImage(float $blackPoint,
                      float $gamma,
                      float $whitePoint,
                      int $channel = Imagick::CHANNEL_ALL): bool;

  /**
   * Stretches with saturation the image intensity
   *
   * @param float $blackPoint - blackPoint   The image black point
   * @param float $whitePoint - whitePoint   The image white point
   *
   * @return bool -
   */
  <<__Native>>
  public function linearStretchImage(float $blackPoint,
                              float $whitePoint): bool;

  /**
   * Animates an image or images
   *
   * @param int $width - width   The width of the target size
   * @param int $height - height   The height of the target size
   * @param float $delta_x - delta_x   How much the seam can traverse on
   *   x-axis. Passing 0 causes the seams to be straight.
   * @param float $rigidity - rigidity   Introduces a bias for
   *   non-straight seams. This parameter is typically 0.
   *
   * @return bool -
   */
  <<__Native>>
  public function liquidRescaleImage(int $width,
                              int $height,
                              float $delta_x,
                              float $rigidity): bool;

  /**
   * Scales an image proportionally 2x
   *
   * @return bool -
   */
  <<__Native>>
  public function magnifyImage(): bool;

  /**
   * Replaces the colors of an image with the closest color from a reference
   * image.
   *
   * @param Imagick $map - map
   * @param bool $dither - dither
   *
   * @return bool -
   */
  <<__Native>>
  public function mapImage(Imagick $map,
                    bool $dither): bool;

  /**
   * Changes the transparency value of a color
   *
   * @param float $alpha - alpha   The level of transparency: 1.0 is
   *   fully opaque and 0.0 is fully transparent.
   * @param float $fuzz - fuzz   The fuzz member of image defines how
   *   much tolerance is acceptable to consider two colors as the same.
   * @param mixed $bordercolor - bordercolor   An ImagickPixel object or
   *   string representing the border color.
   * @param int $x - x   The starting x coordinate of the operation.
   * @param int $y - y   The starting y coordinate of the operation.
   *
   * @return bool -
   */
  <<__Native>>
  public function matteFloodfillImage(float $alpha,
                               float $fuzz,
                               mixed $bordercolor,
                               int $x,
                               int $y): bool;

  /**
   * Applies a digital filter
   *
   * @param float $radius - radius   The radius of the pixel
   *   neighborhood.
   *
   * @return bool -
   */
  <<__Native>>
  public function medianFilterImage(float $radius): bool;

  /**
   * Merges image layers
   *
   * @param int $layer_method - layer_method   One of the
   *   Imagick::LAYERMETHOD_* constants
   *
   * @return Imagick -
   */
  <<__Native>>
  public function mergeImageLayers(int $layer_method): Imagick;

  /**
   * Scales an image proportionally to half its size
   *
   * @return bool -
   */
  <<__Native>>
  public function minifyImage(): bool;

  /**
   * Control the brightness, saturation, and hue
   *
   * @param float $brightness - brightness
   * @param float $saturation - saturation
   * @param float $hue - hue
   *
   * @return bool -
   */
  <<__Native>>
  public function modulateImage(float $brightness,
                         float $saturation,
                         float $hue): bool;

  /**
   * Creates a composite image
   *
   * @param ImagickDraw $draw - draw   The font name, size, and color are
   *   obtained from this object.
   * @param string $tile_geometry - tile_geometry   The number of tiles
   *   per row and page (e.g. 6x4+0+0).
   * @param string $thumbnail_geometry - thumbnail_geometry   Preferred
   *   image size and border size of each thumbnail (e.g. 120x120+4+3>).
   * @param int $mode - mode   Thumbnail framing mode, see Montage Mode
   *   constants.
   * @param string $frame - frame   Surround the image with an ornamental
   *   border (e.g. 15x15+3+3). The frame color is that of the thumbnail's
   *   matte color.
   *
   * @return Imagick -
   */
  <<__Native>>
  public function montageImage(ImagickDraw $draw,
                        string $tile_geometry,
                        string $thumbnail_geometry,
                        int $mode,
                        string $frame): Imagick;

  /**
   * Method morphs a set of images
   *
   * @param int $number_frames - number_frames   The number of in-between
   *   images to generate.
   *
   * @return Imagick - This method returns a new Imagick object on
   *   success.
   */
  <<__Native>>
  public function morphImages(int $number_frames): Imagick;

  /**
   * Forms a mosaic from images
   *
   * @return Imagick -
   */
  <<__Native>>
  public function mosaicImages(): Imagick;

  /**
   * Simulates motion blur
   *
   * @param float $radius - radius   The radius of the Gaussian, in
   *   pixels, not counting the center pixel.
   * @param float $sigma - sigma   The standard deviation of the
   *   Gaussian, in pixels.
   * @param float $angle - angle   Apply the effect along this angle.
   * @param int $channel - channel   Provide any channel constant that is
   *   valid for your channel mode. To apply to more than one channel,
   *   combine channeltype constants using bitwise operators. Refer to this
   *   list of channel constants. The channel argument affects only if
   *   Imagick is compiled against ImageMagick version 6.4.4 or greater.
   *
   * @return bool -
   */
  <<__Native>>
  public function motionBlurImage(float $radius,
                           float $sigma,
                           float $angle,
                           int $channel = Imagick::CHANNEL_DEFAULT): bool;

  /**
   * Negates the colors in the reference image
   *
   * @param bool $gray - gray   Whether to only negate grayscale pixels
   *   within the image.
   * @param int $channel - channel   Provide any channel constant that is
   *   valid for your channel mode. To apply to more than one channel,
   *   combine channeltype constants using bitwise operators. Refer to this
   *   list of channel constants.
   *
   * @return bool -
   */
  <<__Native>>
  public function negateImage(bool $gray,
                       int $channel = Imagick::CHANNEL_ALL): bool;

  /**
   * Creates a new image
   *
   * @param int $cols - cols   Columns in the new image
   * @param int $rows - rows   Rows in the new image
   * @param mixed $background - background   The background color used
   *   for this image
   * @param string $format - format   Image format. This parameter was
   *   added in Imagick version 2.0.1.
   *
   * @return bool -
   */
  <<__Native>>
  public function newImage(int $cols,
                    int $rows,
                    mixed $background,
                    string $format = ''): bool;

  /**
   * Creates a new image
   *
   * @param int $columns - columns   columns in the new image
   * @param int $rows - rows   rows in the new image
   * @param string $pseudoString - pseudoString   string containing
   *   pseudo image definition.
   *
   * @return bool -
   */
  <<__Native>>
  public function newPseudoImage(int $columns,
                          int $rows,
                          string $pseudoString): bool;

  /**
   * Moves to the next image
   *
   * @return bool -
   */
  <<__Native>>
  public function nextImage(): bool;

  /**
   * Enhances the contrast of a color image
   *
   * @param int $channel - channel   Provide any channel constant that is
   *   valid for your channel mode. To apply to more than one channel,
   *   combine channeltype constants using bitwise operators. Refer to this
   *   list of channel constants.
   *
   * @return bool -
   */
  <<__Native>>
  public function normalizeImage(int $channel = Imagick::CHANNEL_ALL): bool;

  /**
   * Simulates an oil painting
   *
   * @param float $radius - radius   The radius of the circular
   *   neighborhood.
   *
   * @return bool -
   */
  <<__Native>>
  public function oilPaintImage(float $radius): bool;

  /**
   * Changes the color value of any pixel that matches target
   *
   * @param mixed $target - target   ImagickPixel object or a string
   *   containing the color to change
   * @param mixed $fill - fill   The replacement color
   * @param float $fuzz - fuzz
   * @param bool $invert - invert   If TRUE paints any pixel that does
   *   not match the target color.
   * @param int $channel - channel
   *
   * @return bool -
   */
  <<__Native>>
  public function opaquePaintImage(mixed $target,
                            mixed $fill,
                            float $fuzz,
                            bool $invert,
                            int $channel = Imagick::CHANNEL_DEFAULT): bool;

  /**
   * Removes repeated portions of images to optimize
   *
   * @return Imagick -
   */
  <<__Native>>
  public function optimizeImageLayers(): Imagick;

  /**
   * Performs an ordered dither
   *
   * @param string $threshold_map - threshold_map   A string containing
   *   the name of the threshold dither map to use
   * @param int $channel - channel   Provide any channel constant that is
   *   valid for your channel mode. To apply to more than one channel,
   *   combine channeltype constants using bitwise operators. Refer to this
   *   list of channel constants.
   *
   * @return bool -
   */
  <<__Native>>
  public function orderedPosterizeImage(string $threshold_map,
                                 int $channel = Imagick::CHANNEL_ALL): bool;

  /**
   * Changes the color value of any pixel that matches target
   *
   * @param mixed $fill - fill   ImagickPixel object or a string
   *   containing the fill color
   * @param float $fuzz - fuzz   The amount of fuzz. For example, set
   *   fuzz to 10 and the color red at intensities of 100 and 102
   *   respectively are now interpreted as the same color for the purposes
   *   of the floodfill.
   * @param mixed $bordercolor - bordercolor   ImagickPixel object or a
   *   string containing the border color
   * @param int $x - x   X start position of the floodfill
   * @param int $y - y   Y start position of the floodfill
   * @param int $channel - channel
   *
   * @return bool -
   */
  <<__Native>>
  public function paintFloodfillImage(mixed $fill,
                               float $fuzz,
                               mixed $bordercolor,
                               int $x,
                               int $y,
                               int $channel = Imagick::CHANNEL_ALL): bool;

  /**
   * Change any pixel that matches color
   *
   * @param mixed $target - target   Change this target color to the fill
   *   color within the image. An ImagickPixel object or a string
   *   representing the target color.
   * @param mixed $fill - fill   An ImagickPixel object or a string
   *   representing the fill color.
   * @param float $fuzz - fuzz   The fuzz member of image defines how
   *   much tolerance is acceptable to consider two colors as the same.
   * @param int $channel - channel   Provide any channel constant that is
   *   valid for your channel mode. To apply to more than one channel,
   *   combine channeltype constants using bitwise operators. Refer to this
   *   list of channel constants.
   *
   * @return bool -
   */
  <<__Native>>
  public function paintOpaqueImage(mixed $target,
                            mixed $fill,
                            float $fuzz,
                            int $channel = Imagick::CHANNEL_ALL): bool;

  /**
   * Changes any pixel that matches color with the color defined by fill
   *
   * @param mixed $target - target   Change this target color to
   *   specified opacity value within the image.
   * @param float $alpha - alpha   The level of transparency: 1.0 is
   *   fully opaque and 0.0 is fully transparent.
   * @param float $fuzz - fuzz   The fuzz member of image defines how
   *   much tolerance is acceptable to consider two colors as the same.
   *
   * @return bool -
   */
  <<__Native>>
  public function paintTransparentImage(mixed $target,
                                 float $alpha,
                                 float $fuzz): bool;

  /**
   * Fetch basic attributes about the image
   *
   * @param string $filename - filename   The filename to read the
   *   information from.
   *
   * @return bool -
   */
  <<__Native>>
  public function pingImage(string $filename): bool;

  /**
   * Quickly fetch attributes
   *
   * @param string $image - image   A string containing the image.
   *
   * @return bool -
   */
  <<__Native>>
  public function pingImageBlob(string $image): bool;

  /**
   * Get basic image attributes in a lightweight manner
   *
   * @param resource $filehandle - filehandle   An open filehandle to the
   *   image.
   * @param string $fileName - fileName   Optional filename for this
   *   image.
   *
   * @return bool -
   */
  <<__Native>>
  public function pingImageFile(resource $filehandle,
                         string $fileName): bool;

  /**
   * Simulates a Polaroid picture
   *
   * @param ImagickDraw $properties - properties   The polaroid
   *   properties
   * @param float $angle - angle   The polaroid angle
   *
   * @return bool -
   */
  <<__Native>>
  public function polaroidImage(ImagickDraw $properties,
                         float $angle): bool;

  /**
   * Reduces the image to a limited number of color level
   *
   * @param int $levels - levels
   * @param bool $dither - dither
   *
   * @return bool -
   */
  <<__Native>>
  public function posterizeImage(int $levels,
                          bool $dither): bool;

  /**
   * Quickly pin-point appropriate parameters for image processing
   *
   * @param int $preview - preview   Preview type. See Preview type
   *   constants
   *
   * @return Imagick -
   */
  <<__Native>>
  public function previewImages(int $preview): Imagick;

  /**
   * Move to the previous image in the object
   *
   * @return bool -
   */
  <<__Native>>
  public function previousImage(): bool;

  /**
   * Adds or removes a profile from an image
   *
   * @param string $name - name
   * @param string $profile - profile
   *
   * @return bool -
   */
  <<__Native>>
  public function profileImage(string $name,
                        string $profile): bool;

  /**
   * Analyzes the colors within a reference image
   *
   * @param int $numberColors - numberColors
   * @param int $colorspace - colorspace
   * @param int $treedepth - treedepth
   * @param bool $dither - dither
   * @param bool $measureError - measureError
   *
   * @return bool -
   */
  <<__Native>>
  public function quantizeImage(int $numberColors,
                         int $colorspace,
                         int $treedepth,
                         bool $dither,
                         bool $measureError): bool;

  /**
   * Analyzes the colors within a sequence of images
   *
   * @param int $numberColors - numberColors
   * @param int $colorspace - colorspace
   * @param int $treedepth - treedepth
   * @param bool $dither - dither
   * @param bool $measureError - measureError
   *
   * @return bool -
   */
  <<__Native>>
  public function quantizeImages(int $numberColors,
                          int $colorspace,
                          int $treedepth,
                          bool $dither,
                          bool $measureError): bool;

  /**
   * Returns an array representing the font metrics
   *
   * @param ImagickDraw $properties - properties   ImagickDraw object
   *   containing font properties
   * @param string $text - text   The text
   * @param bool $multiline - multiline   Multiline parameter. If left
   *   empty it is autodetected
   *
   * @return array - Returns a multi-dimensional array representing the
   *   font metrics.
   */
  <<__Native>>
  public function queryFontMetrics(
    ImagickDraw $properties,
    string $text,
    mixed $multiline = null,
  ): darray<arraykey, darray<string, float>>;

  /**
   * Returns the configured fonts
   *
   * @param string $pattern - pattern   The query pattern
   *
   * @return array - Returns an array containing the configured fonts.
   */
  <<__Native>>
  public static function queryFonts(string $pattern = '*'): varray<mixed>;

  /**
   * Returns formats supported by Imagick
   *
   * @param string $pattern - pattern
   *
   * @return array - Returns an array containing the formats supported by
   *   Imagick.
   */
  <<__Native>>
  public static function queryFormats(string $pattern = '*'): varray<mixed>;

  /**
   * Radial blurs an image
   *
   * @param float $angle - angle
   * @param int $channel - channel
   *
   * @return bool -
   */
  <<__Native>>
  public function radialBlurImage(float $angle,
                           int $channel = Imagick::CHANNEL_ALL): bool;

  /**
   * Creates a simulated 3d button-like effect
   *
   * @param int $width - width
   * @param int $height - height
   * @param int $x - x
   * @param int $y - y
   * @param bool $raise - raise
   *
   * @return bool -
   */
  <<__Native>>
  public function raiseImage(int $width,
                      int $height,
                      int $x,
                      int $y,
                      bool $raise): bool;

  /**
   * Creates a high-contrast, two-color image
   *
   * @param float $low - low   The low point
   * @param float $high - high   The high point
   * @param int $channel - channel   Provide any channel constant that is
   *   valid for your channel mode. To apply to more than one channel,
   *   combine channeltype constants using bitwise operators. Refer to this
   *   list of channel constants.
   *
   * @return bool -
   */
  <<__Native>>
  public function randomThresholdImage(float $low,
                                float $high,
                                int $channel = Imagick::CHANNEL_ALL): bool;

  /**
   * Reads image from filename
   *
   * @param string $filename - filename
   *
   * @return bool -
   */
  <<__Native>>
  public function readImage(string $filename): bool;

  /**
   * Reads image from a binary string
   *
   * @param string $image - image
   * @param string $filename -
   *
   * @return bool -
   */
  <<__Native>>
  public function readImageBlob(string $image,
                         string $filename = ''): bool;

  /**
   * Reads image from open filehandle
   *
   * @param resource $filehandle - filehandle
   * @param string $fileName - fileName
   *
   * @return bool -
   */
  <<__Native>>
  public function readImageFile(resource $filehandle,
                         string $fileName = ''): bool;

  <<__Native>>
  public function readImages(varray<string> $files): bool;

  /**
   * Recolors image
   *
   * @param array $matrix - matrix   The matrix containing the color
   *   values
   *
   * @return bool -
   */
  <<__Native>>
  public function recolorImage(varray<float> $matrix): bool;

  /**
   * Smooths the contours of an image
   *
   * @param float $radius - radius
   *
   * @return bool -
   */
  <<__Native>>
  public function reduceNoiseImage(float $radius): bool;

  /**
   * Remaps image colors
   *
   * @param Imagick $replacement - replacement   An Imagick object
   *   containing the replacement colors
   * @param int $dither - dither   Refer to this list of dither method
   *   constants
   *
   * @return bool -
   */
  <<__Native>>
  public function remapImage(Imagick $replacement,
                             int $dither): bool;

  /**
   * Removes an image from the image list
   *
   * @return bool -
   */
  <<__Native>>
  public function removeImage(): bool;

  /**
   * Removes the named image profile and returns it
   *
   * @param string $name - name
   *
   * @return string - Returns a string containing the profile of the
   *   image.
   */
  <<__Native>>
  public function removeImageProfile(string $name): string;

  /**
   * Resample image to desired resolution
   *
   * @param float $x_resolution - x_resolution
   * @param float $y_resolution - y_resolution
   * @param int $filter - filter
   * @param float $blur - blur
   *
   * @return bool -
   */
  <<__Native>>
  public function resampleImage(float $x_resolution,
                         float $y_resolution,
                         int $filter,
                         float $blur): bool;

  /**
   * Reset image page
   *
   * @param string $page - page   The page definition. For example
   *   7168x5147+0+0
   *
   * @return bool -
   */
  <<__Native>>
  public function resetImagePage(string $page): bool;

  /**
   * Scales an image
   *
   * @param int $columns - columns   Width of the image
   * @param int $rows - rows   Height of the image
   * @param int $filter - filter   Refer to the list of filter constants.
   * @param float $blur - blur   The blur factor where 1 is blurry, 1 is
   *   sharp.
   * @param bool $bestfit - bestfit   Optional fit parameter.
   *
   * @return bool -
   */
  <<__Native>>
  public function resizeImage(int $columns,
                       int $rows,
                       int $filter,
                       float $blur,
                       bool $bestfit = false): bool;

  /**
   * Offsets an image
   *
   * @param int $x - x   The X offset.
   * @param int $y - y   The Y offset.
   *
   * @return bool -
   */
  <<__Native>>
  public function rollImage(int $x,
                     int $y): bool;

  /**
   * Rotates an image
   *
   * @param mixed $background - background   The background color
   * @param float $degrees - degrees   The number of degrees to rotate
   *   the image
   *
   * @return bool -
   */
  <<__Native>>
  public function rotateImage(mixed $background,
                       float $degrees): bool;

  /**
   * Rounds image corners
   *
   * @param float $x_rounding - x_rounding   x rounding
   * @param float $y_rounding - y_rounding   y rounding
   * @param float $stroke_width - stroke_width   stroke width
   * @param float $displace - displace   image displace
   * @param float $size_correction - size_correction   size correction
   *
   * @return bool -
   */
  <<__Native>>
  public function roundCorners(float $x_rounding,
                        float $y_rounding,
                        float $stroke_width = 10.0,
                        float $displace = 5.0,
                        float $size_correction = -6.0): bool;

    <<__Native>>
  public function roundCornersImage(float $x_rounding,
                             float $y_rounding,
                             float $stroke_width = 10.0,
                             float $displace = 5.0,
                             float $size_correction = -6.0): bool;

  /**
   * Scales an image with pixel sampling
   *
   * @param int $columns - columns
   * @param int $rows - rows
   *
   * @return bool -
   */
  <<__Native>>
  public function sampleImage(int $columns,
                       int $rows): bool;

  /**
   * Scales the size of an image
   *
   * @param int $cols - cols
   * @param int $rows - rows
   * @param bool $bestfit - bestfit
   *
   * @return bool -
   */
  <<__Native>>
  public function scaleImage(int $cols,
                      int $rows,
                      bool $bestfit = false): bool;

  /**
   * Segments an image
   *
   * @param int $COLORSPACE - COLORSPACE   One of the COLORSPACE
   *   constants.
   * @param float $cluster_threshold - cluster_threshold   A percentage
   *   describing minimum number of pixels contained in hexedra before it
   *   is considered valid.
   * @param float $smooth_threshold - smooth_threshold   Eliminates noise
   *   from the histogram.
   * @param bool $verbose - verbose   Whether to output detailed
   *   information about recognised classes.
   *
   * @return bool -
   */
  <<__Native>>
  public function segmentImage(int $COLORSPACE,
                               float $cluster_threshold,
                               float $smooth_threshold,
                               bool $verbose = false): bool;

  /**
   * Separates a channel from the image
   *
   * @param int $channel - channel
   *
   * @return bool -
   */
  <<__Native>>
  public function separateImageChannel(int $channel): bool;

  /**
   * Sepia tones an image
   *
   * @param float $threshold - threshold
   *
   * @return bool -
   */
  <<__Native>>
  public function sepiaToneImage(float $threshold): bool;

  /**
   * Sets the object's default background color
   *
   * @param mixed $background - background
   *
   * @return bool -
   */
  <<__Native>>
  public function setBackgroundColor(mixed $background): bool;

  /**
   * Set colorspace
   *
   * @param int $COLORSPACE - COLORSPACE   One of the COLORSPACE
   *   constants
   *
   * @return bool -
   */
  <<__Native>>
  public function setColorspace(int $COLORSPACE): bool;

  /**
   * Sets the object's default compression type
   *
   * @param int $compression - compression
   *
   * @return bool -
   */
  <<__Native>>
  public function setCompression(int $compression): bool;

  /**
   * Sets the object's default compression quality
   *
   * @param int $quality - quality
   *
   * @return bool -
   */
  <<__Native>>
  public function setCompressionQuality(int $quality): bool;

  /**
   * Sets the filename before you read or write the image
   *
   * @param string $filename - filename
   *
   * @return bool -
   */
  <<__Native>>
  public function setFilename(string $filename): bool;

  /**
   * Sets the Imagick iterator to the first image
   *
   * @return bool -
   */
  <<__Native>>
  public function setFirstIterator(): bool;

  /**
   * Sets font
   *
   * @param string $font - font   Font name or a filename
   *
   * @return bool -
   */
  <<__Native>>
  public function setFont(string $font): bool;

  /**
   * Sets the format of the Imagick object
   *
   * @param string $format - format
   *
   * @return bool -
   */
  <<__Native>>
  public function setFormat(string $format): bool;

  /**
   * Sets the gravity
   *
   * @param int $gravity - gravity   The gravity property. Refer to the
   *   list of gravity constants.
   *
   * @return bool -
   */
  <<__Native>>
  public function setGravity(int $gravity): bool;

  /**
   * Replaces image in the object
   *
   * @param Imagick $replace - replace   The replace Imagick object
   *
   * @return bool -
   */
  <<__Native>>
  public function setImage(Imagick $replace): bool;

  /**
   * Sets image alpha channel
   *
   * @param int $mode - mode   One of the Imagick::ALPHACHANNEL_*
   *   constants
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageAlphaChannel(int $mode): bool;

  /**
   * Set image artifact
   *
   * @param string $artifact - artifact   The name of the artifact
   * @param string $value - value   The value of the artifact
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageArtifact(string $artifact,
                            string $value): bool;

  /**
   * Sets the image background color
   *
   * @param mixed $background - background
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageBackgroundColor(mixed $background): bool;

  /**
   * Sets the image bias for any method that convolves an image
   *
   * @param float $bias - bias
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageBias(float $bias): bool;

  /**
   * Sets the image chromaticity blue primary point
   *
   * @param float $x - x
   * @param float $y - y
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageBluePrimary(float $x,
                               float $y): bool;

  /**
   * Sets the image border color
   *
   * @param mixed $border - border   The border color
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageBorderColor(mixed $border): bool;

  /**
   * Sets the depth of a particular image channel
   *
   * @param int $channel - channel
   * @param int $depth - depth
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageChannelDepth(int $channel,
                                int $depth): bool;

  /**
   * Sets image clip mask
   *
   * @param Imagick $clip_mask - clip_mask   The Imagick object
   *   containing the clip mask
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageClipMask(Imagick $clip_mask): bool;

  /**
   * Sets the color of the specified colormap index
   *
   * @param int $index - index
   * @param ImagickPixel $color - color
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageColormapColor(int $index,
                                 ImagickPixel $color): bool;

  /**
   * Sets the image colorspace
   *
   * @param int $colorspace - colorspace   One of the COLORSPACE
   *   constants
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageColorspace(int $colorspace): bool;

  /**
   * Sets the image composite operator
   *
   * @param int $compose - compose
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageCompose(int $compose): bool;

  /**
   * Sets the image compression
   *
   * @param int $compression - compression   One of the COMPRESSION
   *   constants
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageCompression(int $compression): bool;

  /**
   * Sets the image compression quality
   *
   * @param int $quality - quality   The image compression quality as an
   *   integer
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageCompressionQuality(int $quality): bool;

  /**
   * Sets the image delay
   *
   * @param int $delay - delay   The amount of time expressed in 'ticks'
   *   that the image should be displayed for. For animated GIFs there are
   *   100 ticks per second, so a value of 20 would be 20/100 of a second
   *   aka 1/5th of a second.
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageDelay(int $delay): bool;

  /**
   * Sets the image depth
   *
   * @param int $depth - depth
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageDepth(int $depth): bool;

  /**
   * Sets the image disposal method
   *
   * @param int $dispose - dispose
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageDispose(int $dispose): bool;

  /**
   * Sets the image size
   *
   * @param int $columns - columns
   * @param int $rows - rows
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageExtent(int $columns,
                          int $rows): bool;

  /**
   * Sets the filename of a particular image
   *
   * @param string $filename - filename
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageFilename(string $filename): bool;

  /**
   * Sets the format of a particular image
   *
   * @param string $format - format   String presentation of the image
   *   format. Format support depends on the ImageMagick installation.
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageFormat(string $format): bool;

  /**
   * Sets the image gamma
   *
   * @param float $gamma - gamma
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageGamma(float $gamma): bool;

  /**
   * Sets the image gravity
   *
   * @param int $gravity - gravity   The gravity property. Refer to the
   *   list of gravity constants.
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageGravity(int $gravity): bool;

  /**
   * Sets the image chromaticity green primary point
   *
   * @param float $x - x
   * @param float $y - y
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageGreenPrimary(float $x,
                                float $y): bool;

  /**
   * Set the iterator position
   *
   * @param int $index - index   The position to set the iterator to
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageIndex(int $index): bool;

  /**
   * Sets the image compression
   *
   * @param int $interlace_scheme - interlace_scheme
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageInterlaceScheme(int $interlace_scheme): bool;

  /**
   * Sets the image interpolate pixel method
   *
   * @param int $method - method   The method is one of the
   *   Imagick::INTERPOLATE_* constants
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageInterpolateMethod(int $method): bool;

  /**
   * Sets the image iterations
   *
   * @param int $iterations - iterations   The number of iterations the
   *   image should loop over. Set to '0' to loop continuously.
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageIterations(int $iterations): bool;

  /**
   * Sets the image matte channel
   *
   * @param bool $matte - matte   True activates the matte channel and
   *   false disables it.
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageMatte(bool $matte): bool;

  /**
   * Sets the image matte color
   *
   * @param mixed $matte - matte
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageMatteColor(mixed $matte): bool;

  /**
   * Sets the image opacity level
   *
   * @param float $opacity - opacity   The level of transparency: 1.0 is
   *   fully opaque and 0.0 is fully transparent.
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageOpacity(float $opacity): bool;

  /**
   * Sets the image orientation
   *
   * @param int $orientation - orientation   One of the orientation
   *   constants
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageOrientation(int $orientation): bool;

  /**
   * Sets the page geometry of the image
   *
   * @param int $width - width
   * @param int $height - height
   * @param int $x - x
   * @param int $y - y
   *
   * @return bool -
   */
  <<__Native>>
  public function setImagePage(int $width,
                        int $height,
                        int $x,
                        int $y): bool;

  /**
   * Adds a named profile to the Imagick object
   *
   * @param string $name - name
   * @param string $profile - profile
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageProfile(string $name,
                           string $profile): bool;

  /**
   * Sets an image property
   *
   * @param string $name - name
   * @param string $value - value
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageProperty(string $name,
                            string $value): bool;

  /**
   * Sets the image chromaticity red primary point
   *
   * @param float $x - x
   * @param float $y - y
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageRedPrimary(float $x,
                              float $y): bool;

  /**
   * Sets the image rendering intent
   *
   * @param int $rendering_intent - rendering_intent
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageRenderingIntent(int $rendering_intent): bool;

  /**
   * Sets the image resolution
   *
   * @param float $x_resolution - x_resolution
   * @param float $y_resolution - y_resolution
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageResolution(float $x_resolution,
                              float $y_resolution): bool;

  /**
   * Sets the image scene
   *
   * @param int $scene - scene
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageScene(int $scene): bool;

  /**
   * Sets the image ticks-per-second
   *
   * @param int $ticks_per_second - ticks_per_second   The duration for
   *   which an image should be displayed expressed in ticks per second.
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageTicksPerSecond(int $ticks_per_second): bool;

  /**
   * Sets the image type
   *
   * @param int $image_type - image_type
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageType(int $image_type): bool;

  /**
   * Sets the image units of resolution
   *
   * @param int $units - units
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageUnits(int $units): bool;

  /**
   * Sets the image virtual pixel method
   *
   * @param int $method - method
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageVirtualPixelMethod(int $method): bool;

  /**
   * Sets the image chromaticity white point
   *
   * @param float $x - x
   * @param float $y - y
   *
   * @return bool -
   */
  <<__Native>>
  public function setImageWhitePoint(float $x,
                              float $y): bool;

  /**
   * Sets the image compression
   *
   * @param int $interlace_scheme - interlace_scheme
   *
   * @return bool -
   */
  <<__Native>>
  public function setInterlaceScheme(int $interlace_scheme): bool;

  /**
   * Set the iterator position
   *
   * @param int $index - index   The position to set the iterator to
   *
   * @return bool -
   */
  <<__Native>>
  public function setIteratorIndex(int $index): bool;

  /**
   * Sets the Imagick iterator to the last image
   *
   * @return bool -
   */
  <<__Native>>
  public function setLastIterator(): bool;

  /**
   * Set an option
   *
   * @param string $key - key
   * @param string $value - value
   *
   * @return bool -
   */
  <<__Native>>
  public function setOption(string $key,
                     string $value): bool;

  /**
   * Sets the page geometry of the Imagick object
   *
   * @param int $width - width
   * @param int $height - height
   * @param int $x - x
   * @param int $y - y
   *
   * @return bool -
   */
  <<__Native>>
  public function setPage(int $width,
                   int $height,
                   int $x,
                   int $y): bool;

  /**
   * Sets point size
   *
   * @param float $point_size - point_size   Point size
   *
   * @return bool -
   */
  <<__Native>>
  public function setPointSize(float $point_size): bool;

  /**
   * Sets the image resolution
   *
   * @param float $x_resolution - x_resolution   The horizontal
   *   resolution.
   * @param float $y_resolution - y_resolution   The vertical resolution.
   *
   * @return bool -
   */
  <<__Native>>
  public function setResolution(float $x_resolution,
                         float $y_resolution): bool;

  /**
   * Sets the limit for a particular resource in megabytes
   *
   * @param int $type - type   Refer to the list of resourcetype
   *   constants.
   * @param int $limit - limit   The resource limit. The unit depends on
   *   the type of the resource being limited.
   *
   * @return bool -
   */
  <<__Native>>
  public static function setResourceLimit(int $type,
                                   int $limit): bool;

  /**
   * Sets the image sampling factors
   *
   * @param array $factors - factors
   *
   * @return bool -
   */
  <<__Native>>
  public function setSamplingFactors(varray<float> $factors): bool;

  /**
   * Sets the size of the Imagick object
   *
   * @param int $columns - columns
   * @param int $rows - rows
   *
   * @return bool -
   */
  <<__Native>>
  public function setSize(int $columns,
                   int $rows): bool;

  /**
   * Sets the size and offset of the Imagick object
   *
   * @param int $columns - columns   The width in pixels.
   * @param int $rows - rows   The height in pixels.
   * @param int $offset - offset   The image offset.
   *
   * @return bool -
   */
  <<__Native>>
  public function setSizeOffset(int $columns,
                         int $rows,
                         int $offset): bool;

  /**
   * Sets the image type attribute
   *
   * @param int $image_type - image_type
   *
   * @return bool -
   */
  <<__Native>>
  public function setType(int $image_type): bool;

  /**
   * Creates a 3D effect
   *
   * @param bool $gray - gray   A value other than zero shades the
   *   intensity of each pixel.
   * @param float $azimuth - azimuth   Defines the light source
   *   direction.
   * @param float $elevation - elevation   Defines the light source
   *   direction.
   *
   * @return bool -
   */
  <<__Native>>
  public function shadeImage(bool $gray,
                      float $azimuth,
                      float $elevation): bool;

  /**
   * Simulates an image shadow
   *
   * @param float $opacity - opacity
   * @param float $sigma - sigma
   * @param int $x - x
   * @param int $y - y
   *
   * @return bool -
   */
  <<__Native>>
  public function shadowImage(float $opacity,
                       float $sigma,
                       int $x,
                       int $y): bool;

  /**
   * Sharpens an image
   *
   * @param float $radius - radius
   * @param float $sigma - sigma
   * @param int $channel - channel
   *
   * @return bool -
   */
  <<__Native>>
  public function sharpenImage(float $radius,
                        float $sigma,
                        int $channel = Imagick::CHANNEL_ALL): bool;

  /**
   * Shaves pixels from the image edges
   *
   * @param int $columns - columns
   * @param int $rows - rows
   *
   * @return bool -
   */
  <<__Native>>
  public function shaveImage(int $columns,
                      int $rows): bool;

  /**
   * Creating a parallelogram
   *
   * @param mixed $background - background   The background color
   * @param float $x_shear - x_shear   The number of degrees to shear on
   *   the x axis
   * @param float $y_shear - y_shear   The number of degrees to shear on
   *   the y axis
   *
   * @return bool -
   */
  <<__Native>>
  public function shearImage(mixed $background,
                      float $x_shear,
                      float $y_shear): bool;

  /**
   * Adjusts the contrast of an image
   *
   * @param bool $sharpen - sharpen
   * @param float $alpha - alpha
   * @param float $beta - beta
   * @param int $channel - channel
   *
   * @return bool -
   */
  <<__Native>>
  public function sigmoidalContrastImage(bool $sharpen,
                                  float $alpha,
                                  float $beta,
                                  int $channel = Imagick::CHANNEL_ALL): bool;

  /**
   * Simulates a pencil sketch
   *
   * @param float $radius - radius   The radius of the Gaussian, in
   *   pixels, not counting the center pixel
   * @param float $sigma - sigma   The standard deviation of the
   *   Gaussian, in pixels.
   * @param float $angle - angle   Apply the effect along this angle.
   *
   * @return bool -
   */
  <<__Native>>
  public function sketchImage(float $radius,
                       float $sigma,
                       float $angle): bool;

  /**
   * Applies a solarizing effect to the image
   *
   * @param int $threshold - threshold
   *
   * @return bool -
   */
  <<__Native>>
  public function solarizeImage(int $threshold): bool;

  /**
   * Interpolates colors
   *
   * @param int $SPARSE_METHOD - SPARSE_METHOD   Refer to this list of
   *   sparse method constants
   * @param array $arguments - arguments   An array containing the
   *   coordinates. The array is in format array(1,1, 2,45)
   * @param int $channel - CHANNEL
   *
   * @return bool -
   */
  <<__Native>>
  public function sparseColorImage(int $SPARSE_METHOD,
                                   varray<float> $arguments,
                                   int $channel = Imagick::CHANNEL_DEFAULT): bool;

  /**
   * Splices a solid color into the image
   *
   * @param int $width - width
   * @param int $height - height
   * @param int $x - x
   * @param int $y - y
   *
   * @return bool -
   */
  <<__Native>>
  public function spliceImage(int $width,
                       int $height,
                       int $x,
                       int $y): bool;

  /**
   * Randomly displaces each pixel in a block
   *
   * @param float $radius - radius
   *
   * @return bool -
   */
  <<__Native>>
  public function spreadImage(float $radius): bool;

  /**
   * Hides a digital watermark within the image
   *
   * @param Imagick $watermark_wand - watermark_wand
   * @param int $offset - offset
   *
   * @return Imagick -
   */
  <<__Native>>
  public function steganoImage(Imagick $watermark_wand,
                        int $offset): Imagick;

  /**
   * Composites two images
   *
   * @param Imagick $offset_wand - offset_wand
   *
   * @return Imagick -
   */
  <<__Native>>
  public function stereoImage(Imagick $offset_wand): Imagick;

  /**
   * Strips an image of all profiles and comments
   *
   * @return bool -
   */
  <<__Native>>
  public function stripImage(): bool;

  /**
   * Swirls the pixels about the center of the image
   *
   * @param float $degrees - degrees
   *
   * @return bool -
   */
  <<__Native>>
  public function swirlImage(float $degrees): bool;

  /**
   * Repeatedly tiles the texture image
   *
   * @param Imagick $texture_wand - texture_wand
   *
   * @return Imagick -
   */
  <<__Native>>
  public function textureImage(Imagick $texture_wand): Imagick;

  /**
   * Changes the value of individual pixels based on a threshold
   *
   * @param float $threshold - threshold
   * @param int $channel - channel
   *
   * @return bool -
   */
  <<__Native>>
  public function thresholdImage(float $threshold,
                          int $channel = Imagick::CHANNEL_ALL): bool;

  /**
   * Changes the size of an image
   *
   * @param int $columns - columns   Image width
   * @param int $rows - rows   Image height
   * @param bool $bestfit - bestfit   Whether to force maximum values
   * @param bool $fill -
   *
   * @return bool -
   */
  <<__Native>>
  public function thumbnailImage(int $columns,
                          int $rows,
                          bool $bestfit = false,
                          bool $fill = false): bool;

  /**
   * Applies a color vector to each pixel in the image
   *
   * @param mixed $tint - tint
   * @param mixed $opacity - opacity
   *
   * @return bool -
   */
  <<__Native>>
  public function tintImage(mixed $tint,
                     mixed $opacity): bool;

  /**
   * Convenience method for setting crop size and the image geometry
   *
   * @param string $crop - crop   A crop geometry string. This geometry
   *   defines a subregion of the image to crop.
   * @param string $geometry - geometry   An image geometry string. This
   *   geometry defines the final size of the image.
   *
   * @return Imagick -
   */
  <<__Native>>
  public function transformImage(string $crop,
                          string $geometry): Imagick;

  /**
   * Paints pixels transparent
   *
   * @param mixed $target - target   The target color to paint
   * @param float $alpha - alpha
   * @param float $fuzz - fuzz
   * @param bool $invert - invert   If TRUE paints any pixel that does
   *   not match the target color.
   *
   * @return bool -
   */
  <<__Native>>
  public function transparentPaintImage(mixed $target,
                                 float $alpha,
                                 float $fuzz,
                                 bool $invert): bool;

  /**
   * Creates a vertical mirror image
   *
   * @return bool -
   */
  <<__Native>>
  public function transposeImage(): bool;

  /**
   * Creates a horizontal mirror image
   *
   * @return bool -
   */
  <<__Native>>
  public function transverseImage(): bool;

  /**
   * Remove edges from the image
   *
   * @param float $fuzz - fuzz   By default target must match a
   *   particular pixel color exactly. However, in many cases two colors
   *   may differ by a small amount. The fuzz member of image defines how
   *   much tolerance is acceptable to consider two colors as the same.
   *   This parameter represents the variation on the quantum range.
   *
   * @return bool -
   */
  <<__Native>>
  public function trimImage(float $fuzz): bool;

  /**
   * Discards all but one of any pixel color
   *
   * @return bool -
   */
  <<__Native>>
  public function uniqueImageColors(): bool;

  /**
   * Sharpens an image
   *
   * @param float $radius - radius
   * @param float $sigma - sigma
   * @param float $amount - amount
   * @param float $threshold - threshold
   * @param int $channel - channel
   *
   * @return bool -
   */
  <<__Native>>
  public function unsharpMaskImage(float $radius,
                            float $sigma,
                            float $amount,
                            float $threshold,
                            int $channel = Imagick::CHANNEL_ALL): bool;

  /**
   * Checks if the current item is valid
   *
   * @return bool -
   */
  <<__Native>>
  public function valid(): bool;

  /**
   * Adds vignette filter to the image
   *
   * @param float $blackPoint - blackPoint   The black point.
   * @param float $whitePoint - whitePoint   The white point
   * @param int $x - x   X offset of the ellipse
   * @param int $y - y   Y offset of the ellipse
   *
   * @return bool -
   */
  <<__Native>>
  public function vignetteImage(float $blackPoint,
                         float $whitePoint,
                         int $x,
                         int $y): bool;

  /**
   * Applies wave filter to the image
   *
   * @param float $amplitude - amplitude   The amplitude of the wave.
   * @param float $length - length   The length of the wave.
   *
   * @return bool -
   */
  <<__Native>>
  public function waveImage(float $amplitude,
                     float $length): bool;

  /**
   * Force all pixels above the threshold into white
   *
   * @param mixed $threshold - threshold
   *
   * @return bool -
   */
  <<__Native>>
  public function whiteThresholdImage(mixed $threshold): bool;

  /**
   * Writes an image to the specified filename
   *
   * @param string $filename - filename   Filename where to write the
   *   image. The extension of the filename defines the type of the file.
   *   Format can be forced regardless of file extension using format:
   *   prefix, for example "jpg:test.png".
   *
   * @return bool -
   */
  <<__Native>>
  public function writeImage(string $filename = ''): bool;

  /**
   * Writes an image to a filehandle
   *
   * @param resource $filehandle - filehandle   Filehandle where to write
   *   the image
   *
   * @return bool -
   */
  <<__Native>>
  public function writeImageFile(resource $filehandle,
                          string $format = ''): bool;

  /**
   * Writes an image or image sequence
   *
   * @param string $filename - filename
   * @param bool $adjoin - adjoin
   *
   * @return bool -
   */
  <<__Native>>
  public function writeImages(string $filename,
                       bool $adjoin): bool;

  /**
   * Writes frames to a filehandle
   *
   * @param resource $filehandle - filehandle   Filehandle where to write
   *   the images
   *
   * @return bool -
   */
  <<__Native>>
  public function writeImagesFile(resource $filehandle,
                           string $format = ''): bool;

}

class ImagickDraw {
  private ?resource $wand = null;

  /**
   * Adjusts the current affine transformation matrix
   *
   * @param array $affine - Affine matrix parameters
   *
   * @return bool -
   */
  <<__Native>>
  public function affine(darray<string, float> $affine): bool;

  /**
   * Draws text on the image
   *
   * @param float $x - The x coordinate where text is drawn
   * @param float $y - The y coordinate where text is drawn
   * @param string $text - The text to draw on the image
   *
   * @return bool -
   */
  <<__Native>>
  public function annotation(float $x,
                      float $y,
                      string $text): bool;

  /**
   * Draws an arc
   *
   * @param float $sx - Starting x ordinate of bounding rectangle
   * @param float $sy - starting y ordinate of bounding rectangle
   * @param float $ex - ending x ordinate of bounding rectangle
   * @param float $ey - ending y ordinate of bounding rectangle
   * @param float $sd - starting degrees of rotation
   * @param float $ed - ending degrees of rotation
   *
   * @return bool -
   */
  <<__Native>>
  public function arc(float $sx,
               float $sy,
               float $ex,
               float $ey,
               float $sd,
               float $ed): bool;

  /**
   * Draws a bezier curve
   *
   * @param array $coordinates - Multidimensional array like array(
   *   array( 'x' => 1, 'y' => 2 ), array( 'x' => 3, 'y' => 4 ) )
   *
   * @return bool -
   */
  <<__Native>>
  public function bezier(varray<darray<string, float>> $coordinates): bool;

  /**
   * Draws a circle
   *
   * @param float $ox - origin x coordinate
   * @param float $oy - origin y coordinate
   * @param float $px - perimeter x coordinate
   * @param float $py - perimeter y coordinate
   *
   * @return bool -
   */
  <<__Native>>
  public function circle(float $ox,
                  float $oy,
                  float $px,
                  float $py): bool;

  /**
   * Clears the ImagickDraw
   *
   * @return bool - Returns an ImagickDraw object.
   */
  <<__Native>>
  public function clear(): bool;

  /**
   * Makes an exact copy of the specified ImagickDraw object
   *
   * @return ImagickDraw - What the function returns, first on success,
   *   then on failure. See also the return.success; entity
   */
  <<__Native>>
  public function __clone(): void;

  /**
   * Draws color on image
   *
   * @param float $x - x coordinate of the paint
   * @param float $y - y coordinate of the paint
   * @param int $paintMethod - one of the PAINT_ constants
   *
   * @return bool -
   */
  <<__Native>>
  public function color(float $x,
                 float $y,
                 int $paintMethod): bool;

  /**
   * Adds a comment
   *
   * @param string $comment - The comment string to add to vector output
   *   stream
   *
   * @return bool -
   */
  <<__Native>>
  public function comment(string $comment): bool;

  /**
   * Composites an image onto the current image
   *
   * @param int $compose - composition operator. One of COMPOSITE_
   *   constants
   * @param float $x - x coordinate of the top left corner
   * @param float $y - y coordinate of the top left corner
   * @param float $width - width of the composition image
   * @param float $height - height of the composition image
   * @param Imagick $compositeWand - the Imagick object where composition
   *   image is taken from
   *
   * @return bool -
   */
  <<__Native>>
  public function composite(int $compose,
                     float $x,
                     float $y,
                     float $width,
                     float $height,
                     Imagick $compositeWand): bool;

  /**
   * The ImagickDraw constructor
   *
   * @return  -
   */
  <<__Native>>
  public function __construct(): void;

  /**
   * Frees all associated resources
   *
   * @return bool -
   */
  <<__Native>>
  public function destroy(): bool;

  /**
   * Draws an ellipse on the image
   *
   * @param float $ox -
   * @param float $oy -
   * @param float $rx -
   * @param float $ry -
   * @param float $start -
   * @param float $end -
   *
   * @return bool -
   */
  <<__Native>>
  public function ellipse(float $ox,
                   float $oy,
                   float $rx,
                   float $ry,
                   float $start,
                   float $end): bool;

  /**
   * Obtains the current clipping path ID
   *
   * @return string - Returns a string containing the clip path ID or
   *   false if no clip path exists.
   */
  <<__Native>>
  public function getClipPath(): string;

  /**
   * Returns the current polygon fill rule
   *
   * @return int - Returns one of the FILLRULE_ constants.
   */
  <<__Native>>
  public function getClipRule(): int;

  /**
   * Returns the interpretation of clip path units
   *
   * @return int - Returns an int on success.
   */
  <<__Native>>
  public function getClipUnits(): int;

  /**
   * Returns the fill color
   *
   * @return ImagickPixel - Returns an ImagickPixel object.
   */
  <<__Native>>
  public function getFillColor(): ImagickPixel;

  /**
   * Returns the opacity used when drawing
   *
   * @return float - The opacity.
   */
  <<__Native>>
  public function getFillOpacity(): float;

  /**
   * Returns the fill rule
   *
   * @return int - Returns a FILLRULE_ constant
   */
  <<__Native>>
  public function getFillRule(): int;

  /**
   * Returns the font
   *
   * @return string - Returns a string on success and false if no font is
   *   set.
   */
  <<__Native>>
  public function getFont(): string;

  /**
   * Returns the font family
   *
   * @return string - Returns the font family currently selected or false
   *   if font family is not set.
   */
  <<__Native>>
  public function getFontFamily(): string;

  /**
   * Returns the font pointsize
   *
   * @return float - Returns the font size associated with the current
   *   ImagickDraw object.
   */
  <<__Native>>
  public function getFontSize(): float;

  /**
   * Returns the font stretch
   *
   * @return int - Returns the font stretch constant (STRETCH_) associated
   *   with the ImagickDraw object or 0 if no stretch is set.
   */
  <<__Native>>
  public function getFontStretch(): int;

  /**
   * Returns the font style
   *
   * @return int - Returns the font style constant (STYLE_) associated
   *   with the ImagickDraw object or 0 if no style is set.
   */
  <<__Native>>
  public function getFontStyle(): int;

  /**
   * Returns the font weight
   *
   * @return int - Returns an int on success and 0 if no weight is set.
   */
  <<__Native>>
  public function getFontWeight(): int;

  /**
   * Returns the text placement gravity
   *
   * @return int - Returns a GRAVITY_ constant on success and 0 if no
   *   gravity is set.
   */
  <<__Native>>
  public function getGravity(): int;

  /**
   * Returns the current stroke antialias setting
   *
   * @return bool - Returns TRUE if antialiasing is on and false if it is
   *   off.
   */
  <<__Native>>
  public function getStrokeAntialias(): bool;

  /**
   * Returns the color used for stroking object outlines
   *
   * @return ImagickPixel - Returns an ImagickPixel object which
   *   describes the color.
   */
  <<__Native>>
  public function getStrokeColor(): ImagickPixel;

  /**
   * Returns an array representing the pattern of dashes and gaps used to
   * stroke paths
   *
   * @return array - Returns an array on success and empty array if not
   *   set.
   */
  <<__Native>>
  public function getStrokeDashArray(): varray<float>;

  /**
   * Returns the offset into the dash pattern to start the dash
   *
   * @return float - Returns a float representing the offset and 0 if
   *   it's not set.
   */
  <<__Native>>
  public function getStrokeDashOffset(): float;

  /**
   * Returns the shape to be used at the end of open subpaths when they are
   * stroked
   *
   * @return int - Returns one of the LINECAP_ constants or 0 if stroke
   *   linecap is not set.
   */
  <<__Native>>
  public function getStrokeLineCap(): int;

  /**
   * Returns the shape to be used at the corners of paths when they are
   * stroked
   *
   * @return int - Returns one of the LINEJOIN_ constants or 0 if stroke
   *   line join is not set.
   */
  <<__Native>>
  public function getStrokeLineJoin(): int;

  /**
   * Returns the stroke miter limit
   *
   * @return int - Returns an int describing the miter limit and 0 if no
   *   miter limit is set.
   */
  <<__Native>>
  public function getStrokeMiterLimit(): int;

  /**
   * Returns the opacity of stroked object outlines
   *
   * @return float - Returns a double describing the opacity.
   */
  <<__Native>>
  public function getStrokeOpacity(): float;

  /**
   * Returns the width of the stroke used to draw object outlines
   *
   * @return float - Returns a double describing the stroke width.
   */
  <<__Native>>
  public function getStrokeWidth(): float;

  /**
   * Returns the text alignment
   *
   * @return int - Returns one of the ALIGN_ constants and 0 if no align
   *   is set.
   */
  <<__Native>>
  public function getTextAlignment(): int;

  /**
   * Returns the current text antialias setting
   *
   * @return bool - Returns TRUE if text is antialiased and false if not.
   */
  <<__Native>>
  public function getTextAntialias(): bool;

  /**
   * Returns the text decoration
   *
   * @return int - Returns one of the DECORATION_ constants and 0 if no
   *   decoration is set.
   */
  <<__Native>>
  public function getTextDecoration(): int;

  /**
   * Returns the code set used for text annotations
   *
   * @return string - Returns a string specifying the code set or false
   *   if text encoding is not set.
   */
  <<__Native>>
  public function getTextEncoding(): string;

  /**
   * Returns the text under color
   *
   * @return ImagickPixel - Returns an ImagickPixel object describing the
   *   color.
   */
  <<__Native>>
  public function getTextUnderColor(): ImagickPixel;

  /**
   * Returns a string containing vector graphics
   *
   * @return string - Returns a string containing the vector graphics.
   */
  <<__Native>>
  public function getVectorGraphics(): string;

  /**
   * Draws a line
   *
   * @param float $sx - starting x coordinate
   * @param float $sy - starting y coordinate
   * @param float $ex - ending x coordinate
   * @param float $ey - ending y coordinate
   *
   * @return bool -
   */
  <<__Native>>
  public function line(float $sx,
                float $sy,
                float $ex,
                float $ey): bool;

  /**
   * Paints on the image's opacity channel
   *
   * @param float $x - x coordinate of the matte
   * @param float $y - y coordinate of the matte
   * @param int $paintMethod - PAINT_ constant
   *
   * @return bool -
   */
  <<__Native>>
  public function matte(float $x,
                 float $y,
                 int $paintMethod): bool;

  /**
   * Adds a path element to the current path
   *
   * @return bool -
   */
  <<__Native>>
  public function pathClose(): bool;

  /**
   * Draws a cubic Bezier curve
   *
   * @param float $x1 - x coordinate of the first control point
   * @param float $y1 - y coordinate of the first control point
   * @param float $x2 - x coordinate of the second control point
   * @param float $y2 - y coordinate of the first control point
   * @param float $x - x coordinate of the curve end
   * @param float $y - y coordinate of the curve end
   *
   * @return bool -
   */
  <<__Native>>
  public function pathCurveToAbsolute(float $x1,
                               float $y1,
                               float $x2,
                               float $y2,
                               float $x,
                               float $y): bool;

  /**
   * Draws a quadratic Bezier curve
   *
   * @param float $x1 - x coordinate of the control point
   * @param float $y1 - y coordinate of the control point
   * @param float $x - x coordinate of the end point
   * @param float $y - y coordinate of the end point
   *
   * @return bool -
   */
  <<__Native>>
  public function pathCurveToQuadraticBezierAbsolute(float $x1,
                                              float $y1,
                                              float $x,
                                              float $y): bool;

  /**
   * Draws a quadratic Bezier curve
   *
   * @param float $x1 - starting x coordinate
   * @param float $y1 - starting y coordinate
   * @param float $x - ending x coordinate
   * @param float $y - ending y coordinate
   *
   * @return bool -
   */
  <<__Native>>
  public function pathCurveToQuadraticBezierRelative(float $x1,
                                              float $y1,
                                              float $x,
                                              float $y): bool;

  /**
   * Draws a quadratic Bezier curve
   *
   * @param float $x - ending x coordinate
   * @param float $y - ending y coordinate
   *
   * @return bool -
   */
  <<__Native>>
  public function pathCurveToQuadraticBezierSmoothAbsolute(float $x,
                                                    float $y): bool;

  /**
   * Draws a quadratic Bezier curve
   *
   * @param float $x - ending x coordinate
   * @param float $y - ending y coordinate
   *
   * @return bool -
   */
  <<__Native>>
  public function pathCurveToQuadraticBezierSmoothRelative(float $x,
                                                    float $y): bool;

  /**
   * Draws a cubic Bezier curve
   *
   * @param float $x1 - x coordinate of starting control point
   * @param float $y1 - y coordinate of starting control point
   * @param float $x2 - x coordinate of ending control point
   * @param float $y2 - y coordinate of ending control point
   * @param float $x - ending x coordinate
   * @param float $y - ending y coordinate
   *
   * @return bool -
   */
  <<__Native>>
  public function pathCurveToRelative(float $x1,
                               float $y1,
                               float $x2,
                               float $y2,
                               float $x,
                               float $y): bool;

  /**
   * Draws a cubic Bezier curve
   *
   * @param float $x2 - x coordinate of the second control point
   * @param float $y2 - y coordinate of the second control point
   * @param float $x - x coordinate of the ending point
   * @param float $y - y coordinate of the ending point
   *
   * @return bool -
   */
  <<__Native>>
  public function pathCurveToSmoothAbsolute(float $x2,
                                     float $y2,
                                     float $x,
                                     float $y): bool;

  /**
   * Draws a cubic Bezier curve
   *
   * @param float $x2 - x coordinate of the second control point
   * @param float $y2 - y coordinate of the second control point
   * @param float $x - x coordinate of the ending point
   * @param float $y - y coordinate of the ending point
   *
   * @return bool -
   */
  <<__Native>>
  public function pathCurveToSmoothRelative(float $x2,
                                     float $y2,
                                     float $x,
                                     float $y): bool;

  /**
   * Draws an elliptical arc
   *
   * @param float $rx - x radius
   * @param float $ry - y radius
   * @param float $x_axis_rotation - x axis rotation
   * @param bool $large_arc_flag - large arc flag
   * @param bool $sweep_flag - sweep flag
   * @param float $x - x coordinate
   * @param float $y - y coordinate
   *
   * @return bool -
   */
  <<__Native>>
  public function pathEllipticArcAbsolute(float $rx,
                                   float $ry,
                                   float $x_axis_rotation,
                                   bool $large_arc_flag,
                                   bool $sweep_flag,
                                   float $x,
                                   float $y): bool;

  /**
   * Draws an elliptical arc
   *
   * @param float $rx - x radius
   * @param float $ry - y radius
   * @param float $x_axis_rotation - x axis rotation
   * @param bool $large_arc_flag - large arc flag
   * @param bool $sweep_flag - sweep flag
   * @param float $x - x coordinate
   * @param float $y - y coordinate
   *
   * @return bool -
   */
  <<__Native>>
  public function pathEllipticArcRelative(float $rx,
                                   float $ry,
                                   float $x_axis_rotation,
                                   bool $large_arc_flag,
                                   bool $sweep_flag,
                                   float $x,
                                   float $y): bool;

  /**
   * Terminates the current path
   *
   * @return bool -
   */
  <<__Native>>
  public function pathFinish(): bool;

  /**
   * Draws a line path
   *
   * @param float $x - starting x coordinate
   * @param float $y - ending x coordinate
   *
   * @return bool -
   */
  <<__Native>>
  public function pathLineToAbsolute(float $x,
                              float $y): bool;

  /**
   * Draws a horizontal line path
   *
   * @param float $x - x coordinate
   *
   * @return bool -
   */
  <<__Native>>
  public function pathLineToHorizontalAbsolute(float $x): bool;

  /**
   * Draws a horizontal line
   *
   * @param float $x - x coordinate
   *
   * @return bool -
   */
  <<__Native>>
  public function pathLineToHorizontalRelative(float $x): bool;

  /**
   * Draws a line path
   *
   * @param float $x - starting x coordinate
   * @param float $y - starting y coordinate
   *
   * @return bool -
   */
  <<__Native>>
  public function pathLineToRelative(float $x,
                              float $y): bool;

  /**
   * Draws a vertical line
   *
   * @param float $y - y coordinate
   *
   * @return bool -
   */
  <<__Native>>
  public function pathLineToVerticalAbsolute(float $y): bool;

  /**
   * Draws a vertical line path
   *
   * @param float $y - y coordinate
   *
   * @return bool -
   */
  <<__Native>>
  public function pathLineToVerticalRelative(float $y): bool;

  /**
   * Starts a new sub-path
   *
   * @param float $x - x coordinate of the starting point
   * @param float $y - y coordinate of the starting point
   *
   * @return bool -
   */
  <<__Native>>
  public function pathMoveToAbsolute(float $x,
                              float $y): bool;

  /**
   * Starts a new sub-path
   *
   * @param float $x - target x coordinate
   * @param float $y - target y coordinate
   *
   * @return bool -
   */
  <<__Native>>
  public function pathMoveToRelative(float $x,
                              float $y): bool;

  /**
   * Declares the start of a path drawing list
   *
   * @return bool -
   */
  <<__Native>>
  public function pathStart(): bool;

  /**
   * Draws a point
   *
   * @param float $x - point's x coordinate
   * @param float $y - point's y coordinate
   *
   * @return bool -
   */
  <<__Native>>
  public function point(float $x,
                 float $y): bool;

  /**
   * Draws a polygon
   *
   * @param array $coordinates - multidimensional array like array(
   *   array( 'x' => 3, 'y' => 4 ), array( 'x' => 2, 'y' => 6 ) );
   *
   * @return bool -
   */
  <<__Native>>
  public function polygon(varray<darray<string, float>> $coordinates): bool;

  /**
   * Draws a polyline
   *
   * @param array $coordinates - array of x and y coordinates: array(
   *   array( 'x' => 4, 'y' => 6 ), array( 'x' => 8, 'y' => 10 ) )
   *
   * @return bool -
   */
  <<__Native>>
  public function polyline(varray<darray<string, float>> $coordinates): bool;

  /**
   * Destroys the current ImagickDraw in the stack, and returns to the
   * previously pushed ImagickDraw
   *
   * @return bool - Returns TRUE on success and false on failure.
   */
  <<__Native>>
  public function pop(): bool;

  /**
   * Terminates a clip path definition
   *
   * @return bool -
   */
  <<__Native>>
  public function popClipPath(): bool;

  /**
   * Terminates a definition list
   *
   * @return bool -
   */
  <<__Native>>
  public function popDefs(): bool;

  /**
   * Terminates a pattern definition
   *
   * @return bool -
   */
  <<__Native>>
  public function popPattern(): bool;

  /**
   * Clones the current ImagickDraw and pushes it to the stack
   *
   * @return bool -
   */
  <<__Native>>
  public function push(): bool;

  /**
   * Starts a clip path definition
   *
   * @param string $clip_mask_id - Clip mask Id
   *
   * @return bool -
   */
  <<__Native>>
  public function pushClipPath(string $clip_mask_id): bool;

  /**
   * Indicates that following commands create named elements for early
   * processing
   *
   * @return bool -
   */
  <<__Native>>
  public function pushDefs(): bool;

  /**
   * Indicates that subsequent commands up to a ImagickDraw::opPattern()
   * command comprise the definition of a named pattern
   *
   * @param string $pattern_id - the pattern Id
   * @param float $x - x coordinate of the top-left corner
   * @param float $y - y coordinate of the top-left corner
   * @param float $width - width of the pattern
   * @param float $height - height of the pattern
   *
   * @return bool -
   */
  <<__Native>>
  public function pushPattern(string $pattern_id,
                       float $x,
                       float $y,
                       float $width,
                       float $height): bool;

  /**
   * Draws a rectangle
   *
   * @param float $x1 - x coordinate of the top left corner
   * @param float $y1 - y coordinate of the top left corner
   * @param float $x2 - x coordinate of the bottom right corner
   * @param float $y2 - y coordinate of the bottom right corner
   *
   * @return bool -
   */
  <<__Native>>
  public function rectangle(float $x1,
                     float $y1,
                     float $x2,
                     float $y2): bool;

  /**
   * Renders all preceding drawing commands onto the image
   *
   * @return bool -
   */
  <<__Native>>
  public function render(): bool;

  /**
   * Applies the specified rotation to the current coordinate space
   *
   * @param float $degrees - degrees to rotate
   *
   * @return bool -
   */
  <<__Native>>
  public function rotate(float $degrees): bool;

  /**
   * Draws a rounded rectangle
   *
   * @param float $x1 - x coordinate of the top left corner
   * @param float $y1 - y coordinate of the top left corner
   * @param float $x2 - x coordinate of the bottom right
   * @param float $y2 - y coordinate of the bottom right
   * @param float $rx - x rounding
   * @param float $ry - y rounding
   *
   * @return bool -
   */
  <<__Native>>
  public function roundRectangle(float $x1,
                          float $y1,
                          float $x2,
                          float $y2,
                          float $rx,
                          float $ry): bool;

  /**
   * Adjusts the scaling factor
   *
   * @param float $x - horizontal factor
   * @param float $y - vertical factor
   *
   * @return bool -
   */
  <<__Native>>
  public function scale(float $x,
                 float $y): bool;

  /**
   * Associates a named clipping path with the image
   *
   * @param string $clip_mask - the clipping path name
   *
   * @return bool -
   */
  <<__Native>>
  public function setClipPath(string $clip_mask): bool;

  /**
   * Set the polygon fill rule to be used by the clipping path
   *
   * @param int $fill_rule - FILLRULE_ constant
   *
   * @return bool -
   */
  <<__Native>>
  public function setClipRule(int $fill_rule): bool;

  /**
   * Sets the interpretation of clip path units
   *
   * @param int $clip_units - the number of clip units
   *
   * @return bool -
   */
  <<__Native>>
  public function setClipUnits(int $clip_units): bool;

  /**
   * Sets the opacity to use when drawing using the fill color or fill texture
   *
   * @param float $opacity - fill alpha
   *
   * @return bool -
   */
  <<__Native>>
  public function setFillAlpha(float $opacity): bool;

  /**
   * Sets the fill color to be used for drawing filled objects
   *
   * @param ImagickPixel $fill_pixel - ImagickPixel to use to set the
   *   color
   *
   * @return bool -
   */
  <<__Native>>
  public function setFillColor(mixed $fill_pixel): bool;

  /**
   * Sets the opacity to use when drawing using the fill color or fill texture
   *
   * @param float $fillOpacity - the fill opacity
   *
   * @return bool -
   */
  <<__Native>>
  public function setFillOpacity(float $fillOpacity): bool;

  /**
   * Sets the URL to use as a fill pattern for filling objects
   *
   * @param string $fill_url - URL to use to obtain fill pattern.
   *
   * @return bool -
   */
  <<__Native>>
  public function setFillPatternURL(string $fill_url): bool;

  /**
   * Sets the fill rule to use while drawing polygons
   *
   * @param int $fill_rule - FILLRULE_ constant
   *
   * @return bool -
   */
  <<__Native>>
  public function setFillRule(int $fill_rule): bool;

  /**
   * Sets the fully-specified font to use when annotating with text
   *
   * @param string $font_name -
   *
   * @return bool -
   */
  <<__Native>>
  public function setFont(string $font_name): bool;

  /**
   * Sets the font family to use when annotating with text
   *
   * @param string $font_family - the font family
   *
   * @return bool -
   */
  <<__Native>>
  public function setFontFamily(string $font_family): bool;

  /**
   * Sets the font pointsize to use when annotating with text
   *
   * @param float $pointsize - the point size
   *
   * @return bool -
   */
  <<__Native>>
  public function setFontSize(float $pointsize): bool;

  /**
   * Sets the font stretch to use when annotating with text
   *
   * @param int $fontStretch - STRETCH_ constant
   *
   * @return bool -
   */
  <<__Native>>
  public function setFontStretch(int $fontStretch): bool;

  /**
   * Sets the font style to use when annotating with text
   *
   * @param int $style - STYLETYPE_ constant
   *
   * @return bool -
   */
  <<__Native>>
  public function setFontStyle(int $style): bool;

  /**
   * Sets the font weight
   *
   * @param int $font_weight -
   *
   * @return bool -
   */
  <<__Native>>
  public function setFontWeight(int $font_weight): bool;

  /**
   * Sets the text placement gravity
   *
   * @param int $gravity - GRAVITY_ constant
   *
   * @return bool -
   */
  <<__Native>>
  public function setGravity(int $gravity): bool;

  <<__Native>>
  public function setResolution(float $x,
                         float $y): bool;

  /**
   * Specifies the opacity of stroked object outlines
   *
   * @param float $opacity - opacity
   *
   * @return bool -
   */
  <<__Native>>
  public function setStrokeAlpha(float $opacity): bool;

  /**
   * Controls whether stroked outlines are antialiased
   *
   * @param bool $stroke_antialias - the antialias setting
   *
   * @return bool -
   */
  <<__Native>>
  public function setStrokeAntialias(bool $stroke_antialias): bool;

  /**
   * Sets the color used for stroking object outlines
   *
   * @param ImagickPixel $stroke_pixel - the stroke color
   *
   * @return bool -
   */
  <<__Native>>
  public function setStrokeColor(mixed $stroke_pixel): bool;

  /**
   * Specifies the pattern of dashes and gaps used to stroke paths
   *
   * @param array $dashArray - array of floats
   *
   * @return bool -
   */
  <<__Native>>
  public function setStrokeDashArray(varray<float> $dashArray): bool;

  /**
   * Specifies the offset into the dash pattern to start the dash
   *
   * @param float $dash_offset - dash offset
   *
   * @return bool -
   */
  <<__Native>>
  public function setStrokeDashOffset(float $dash_offset): bool;

  /**
   * Specifies the shape to be used at the end of open subpaths when they are
   * stroked
   *
   * @param int $linecap - LINECAP_ constant
   *
   * @return bool -
   */
  <<__Native>>
  public function setStrokeLineCap(int $linecap): bool;

  /**
   * Specifies the shape to be used at the corners of paths when they are
   * stroked
   *
   * @param int $linejoin - LINEJOIN_ constant
   *
   * @return bool -
   */
  <<__Native>>
  public function setStrokeLineJoin(int $linejoin): bool;

  /**
   * Specifies the miter limit
   *
   * @param int $miterlimit - the miter limit
   *
   * @return bool -
   */
  <<__Native>>
  public function setStrokeMiterLimit(int $miterlimit): bool;

  /**
   * Specifies the opacity of stroked object outlines
   *
   * @param float $stroke_opacity - stroke opacity. 1.0 is fully opaque
   *
   * @return bool -
   */
  <<__Native>>
  public function setStrokeOpacity(float $stroke_opacity): bool;

  /**
   * Sets the pattern used for stroking object outlines
   *
   * @param string $stroke_url - stroke URL
   *
   * @return bool - Imagick.ImagickDraw.return.success;
   */
  <<__Native>>
  public function setStrokePatternURL(string $stroke_url): bool;

  /**
   * Sets the width of the stroke used to draw object outlines
   *
   * @param float $stroke_width - stroke width
   *
   * @return bool -
   */
  <<__Native>>
  public function setStrokeWidth(float $stroke_width): bool;

  /**
   * Specifies a text alignment
   *
   * @param int $alignment - ALIGN_ constant
   *
   * @return bool -
   */
  <<__Native>>
  public function setTextAlignment(int $alignment): bool;

  /**
   * Controls whether text is antialiased
   *
   * @param bool $antiAlias -
   *
   * @return bool -
   */
  <<__Native>>
  public function setTextAntialias(bool $antiAlias): bool;

  /**
   * Specifies a decoration
   *
   * @param int $decoration - DECORATION_ constant
   *
   * @return bool -
   */
  <<__Native>>
  public function setTextDecoration(int $decoration): bool;

  /**
   * Specifies specifies the text code set
   *
   * @param string $encoding - the encoding name
   *
   * @return bool -
   */
  <<__Native>>
  public function setTextEncoding(string $encoding): bool;

  /**
   * Specifies the color of a background rectangle
   *
   * @param ImagickPixel $under_color - the under color
   *
   * @return bool -
   */
  <<__Native>>
  public function setTextUnderColor(mixed $under_color): bool;

  /**
   * Sets the vector graphics
   *
   * @param string $xml - xml containing the vector graphics
   *
   * @return bool -
   */
  <<__Native>>
  public function setVectorGraphics(string $xml): bool;

  /**
   * Sets the overall canvas size
   *
   * @param int $x1 - left x coordinate
   * @param int $y1 - left y coordinate
   * @param int $x2 - right x coordinate
   * @param int $y2 - right y coordinate
   *
   * @return bool -
   */
  <<__Native>>
  public function setViewbox(int $x1,
                      int $y1,
                      int $x2,
                      int $y2): bool;

  /**
   * Skews the current coordinate system in the horizontal direction
   *
   * @param float $degrees - degrees to skew
   *
   * @return bool -
   */
  <<__Native>>
  public function skewX(float $degrees): bool;

  /**
   * Skews the current coordinate system in the vertical direction
   *
   * @param float $degrees - degrees to skew
   *
   * @return bool -
   */
  <<__Native>>
  public function skewY(float $degrees): bool;

  /**
   * Applies a translation to the current coordinate system
   *
   * @param float $x - horizontal translation
   * @param float $y - vertical translation
   *
   * @return bool -
   */
  <<__Native>>
  public function translate(float $x,
                     float $y): bool;

}

class ImagickPixel {
  private ?resource $wand = null;

  /**
   * Clears resources associated with this object
   *
   * @return bool -
   */
  <<__Native>>
  public function clear(): bool;

  /**
   * The ImagickPixel constructor
   *
   * @param string $color - The optional color string to use as the
   *   initial value of this object.
   *
   * @return  - Returns an ImagickPixel object on success, throwing
   *   ImagickPixelException on failure.
   */
  <<__Native>>
  public function __construct(string $color = ''): void;

  /**
   * Deallocates resources associated with this object
   *
   * @return bool -
   */
  <<__Native>>
  public function destroy(): bool;

  /**
   * Returns the color
   *
   * @param bool $normalized - Normalize the color values
   *
   * @return array - An array of channel values, each normalized if TRUE
   *   is given as param. Throws ImagickPixelException on error.
   */
  <<__Native>>
  public function getColor(bool $normalized = false): darray<string, num>;

  /**
   * Returns the color as a string
   *
   * @return string - Returns the color of the ImagickPixel object as a
   *   string.
   */
  <<__Native>>
  public function getColorAsString(): string;

  /**
   * Returns the color count associated with this color
   *
   * @return int - Returns the color count as an integer on success,
   *   throws ImagickPixelException on failure.
   */
  <<__Native>>
  public function getColorCount(): int;

  /**
   * Gets the normalized value of the provided color channel
   *
   * @param int $color - The color to get the value of, specified as one
   *   of the Imagick color constants. This can be one of the RGB colors,
   *   CMYK colors, alpha and opacity e.g (Imagick::COLOR_BLUE,
   *   Imagick::COLOR_MAGENTA).
   *
   * @return float - The value of the channel, as a normalized
   *   floating-point number, throwing ImagickPixelException on error.
   */
  <<__Native>>
  public function getColorValue(int $color): float;

  /**
   * Returns the normalized HSL color of the ImagickPixel object
   *
   * @return array - Returns the HSL value in an array with the keys
   *   "hue", "saturation", and "luminosity". Throws ImagickPixelException
   *   on failure.
   */
  <<__Native>>
  public function getHSL(): darray<string, float>;

  /**
   * Check the distance between this color and another
   *
   * @param ImagickPixel $color - The ImagickPixel object to compare this
   *   object against.
   * @param float $fuzz - The maximum distance within which to consider
   *   these colors as similar. The theoretical maximum for this value is
   *   the square root of three (1.732).
   *
   * @return bool -
   */
  <<__Native>>
  public function isPixelSimilar(mixed $color,
                          float $fuzz): bool;

  /**
   * Check the distance between this color and another
   *
   * @param ImagickPixel $color - The ImagickPixel object to compare this
   *   object against.
   * @param float $fuzz - The maximum distance within which to consider
   *   these colors as similar. The theoretical maximum for this value is
   *   the square root of three (1.732).
   *
   * @return bool -
   */
  <<__Native>>
  public function isSimilar(mixed $color,
                     float $fuzz): bool;

  /**
   * Sets the color
   *
   * @param string $color - The color definition to use in order to
   *   initialise the ImagickPixel object.
   *
   * @return bool - Returns TRUE if the specified color was set, FALSE
   *   otherwise.
   */
  <<__Native>>
  public function setColor(string $color): bool;

  /**
   * Sets the normalized value of one of the channels
   *
   * @param int $color - One of the Imagick color constants e.g.
   *   \Imagick::COLOR_GREEN or \Imagick::COLOR_ALPHA.
   * @param float $value - The value to set this channel to, ranging from
   *   0 to 1.
   *
   * @return bool -
   */
  <<__Native>>
  public function setColorValue(int $color,
                         float $value): bool;

  /**
   * Sets the normalized HSL color
   *
   * @param float $hue - The normalized value for hue, described as a
   *   fractional arc (between 0 and 1) of the hue circle, where the zero
   *   value is red.
   * @param float $saturation - The normalized value for saturation, with
   *   1 as full saturation.
   * @param float $luminosity - The normalized value for luminosity, on a
   *   scale from black at 0 to white at 1, with the full HS value at 0.5
   *   luminosity.
   *
   * @return bool -
   */
  <<__Native>>
  public function setHSL(float $hue,
                  float $saturation,
                  float $luminosity): bool;

}

class ImagickPixelIterator implements Iterator {
  private ?resource $wand = null;

  <<__Native>>
  public static function getPixelIterator(Imagick $wand): ImagickPixelIterator;

  <<__Native>>
  public static function getPixelRegionIterator(Imagick $wand,
                                         int $x,
                                         int $y,
                                         int $columns,
                                         int $rows): ImagickPixelIterator;

  <<__Native>>
  public function current(): varray<ImagickPixel>;

  <<__Native>>
  public function key(): int;

  <<__Native>>
  public function next(): void;

  <<__Native>>
  public function rewind(): void;

  <<__Native>>
  public function valid(): bool;

  /**
   * Clear resources associated with a PixelIterator
   *
   * @return bool -
   */
  <<__Native>>
  public function clear(): bool;

  /**
   * The ImagickPixelIterator constructor
   *
   * @param Imagick $wand -
   *
   * @return  -
   */
  <<__Native>>
  public function __construct(Imagick $wand): void;

  /**
   * Deallocates resources associated with a PixelIterator
   *
   * @return bool -
   */
  <<__Native>>
  public function destroy(): bool;

  /**
   * Returns the current row of ImagickPixel objects
   *
   * @return array - Returns a row as an array of ImagickPixel objects
   *   that can themselves be iterated.
   */
  <<__Native>>
  public function getCurrentIteratorRow(): varray<ImagickPixel>;

  /**
   * Returns the current pixel iterator row
   *
   * @return int - Returns the integer offset of the row, throwing
   *   ImagickPixelIteratorException on error.
   */
  <<__Native>>
  public function getIteratorRow(): int;

  /**
   * Returns the next row of the pixel iterator
   *
   * @return array - Returns the next row as an array of ImagickPixel
   *   objects, throwing ImagickPixelIteratorException on error.
   */
  <<__Native>>
  public function getNextIteratorRow(): varray<ImagickPixel>;

  /**
   * Returns the previous row
   *
   * @return array - Returns the previous row as an array of
   *   ImagickPixelWand objects from the ImagickPixelIterator, throwing
   *   ImagickPixelIteratorException on error.
   */
  <<__Native>>
  public function getPreviousIteratorRow(): varray<ImagickPixel>;

  /**
   * Returns a new pixel iterator
   *
   * @param Imagick $wand -
   *
   * @return bool - Throwing ImagickPixelIteratorException.
   */
  <<__Native>>
  public function newPixelIterator(Imagick $wand): bool;

  /**
   * Returns a new pixel iterator
   *
   * @param Imagick $wand -
   * @param int $x -
   * @param int $y -
   * @param int $columns -
   * @param int $rows -
   *
   * @return bool - Returns a new ImagickPixelIterator on success; on
   *   failure, throws ImagickPixelIteratorException.
   */
  <<__Native>>
  public function newPixelRegionIterator(Imagick $wand,
                                  int $x,
                                  int $y,
                                  int $columns,
                                  int $rows): bool;

  /**
   * Resets the pixel iterator
   *
   * @return bool -
   */
  <<__Native>>
  public function resetIterator(): bool;

  /**
   * Sets the pixel iterator to the first pixel row
   *
   * @return bool -
   */
  <<__Native>>
  public function setIteratorFirstRow(): bool;

  /**
   * Sets the pixel iterator to the last pixel row
   *
   * @return bool -
   */
  <<__Native>>
  public function setIteratorLastRow(): bool;

  /**
   * Set the pixel iterator row
   *
   * @param int $row -
   *
   * @return bool -
   */
  <<__Native>>
  public function setIteratorRow(int $row): bool;

  /**
   * Syncs the pixel iterator
   *
   * @return bool -
   */
  <<__Native>>
  public function syncIterator(): bool;

}
