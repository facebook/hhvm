<?hh

const bool GD_BUNDLED = true;

/* Gets information about the version and capabilities of the installed GD
 * library.
 */
<<__Native>>
function gd_info(): shape(
  'GD Version' => string,
  'FreeType Support' => bool,
  'FreeType Linkage' => string,
  'T1Lib Support' => bool,
  'GIF Read Support' => bool,
  'GIF Create Support' => bool,
  'JPG Support' => bool,
  'PNG Support' => bool,
  'WBMP Support' => bool,
  'XPM Support' => bool,
  'XBM Support' => bool,
  'JIS-mapped Japanese Font Support' => bool,
);

/* The getimagesize() function will determine the size of any given image file
 * and return the dimensions along with the file type and a height/width text
 * string to be used inside a normal HTML IMG tag and the correspondant HTTP
 * content type.  getimagesize() can also return some more information in
 * imageinfo parameter. Note that JPC and JP2 are capable of having components
 * with different bit depths. In this case, the value for "bits" is the
 * highest bit depth encountered. Also, JP2 files may contain multiple JPEG
 * 2000 codestreams. In this case, getimagesize() returns the values for the
 * first codestream it encounters in the root of the file. The information
 * about icons are retrieved from the icon with the highest bitrate.
 */
<<__Native>>
function getimagesize(string $filename,
                      <<__OutOnly("darray")>>
                      inout mixed $imageinfo): mixed;

/* Identical to getimagesize() except that getimagesizefromstring() accepts
 * a string instead of a file name as the first parameter.
 */
<<__Native>>
function getimagesizefromstring(string $filename,
                      <<__OutOnly("darray")>>
                      inout mixed $imageinfo): mixed;

/* Returns the extension for the given IMAGETYPE_XXX constant.
 */
<<__Native>>
function image_type_to_extension(int $imagetype,
                                 bool $include_dot = true): mixed;

/* The image_type_to_mime_type() function will determine the Mime-Type for an
 * IMAGETYPE constant.
 */
<<__Native>>
function image_type_to_mime_type(int $imagetype): string;

/* image2wbmp() outputs or save a WBMP version of the given image.
 */
<<__Native>>
function image2wbmp(resource $image,
                    string $filename = "",
                    int $threshold = -1): bool;

/* Return an image containing the affine tramsformed src image, using
 * an optional clipping area.
 */
<<__Native>>
function imageaffine(
  resource $image,
  varray<mixed> $affine = varray[],
  shape(
    ?'x' => int,
    ?'y' => int,
    ?'width' => int,
    ?'height' => int,
  ) $clip = shape()
): mixed;

/* Concat two matrices.
 */
<<__Native>>
function imageaffinematrixconcat(varray<mixed> $m1, varray<mixed> $m2): mixed;

/* Return an image containing the affine tramsformed src image, using
 * an optional clipping area.
 */
<<__Native>>
function imageaffinematrixget(int $type, mixed $options = darray[]): mixed;

/* imagealphablending() allows for two different modes of drawing on truecolor
 * images. In blending mode, the alpha channel component of the color supplied
 * to all drawing function, such as imagesetpixel() determines how much of the
 * underlying color should be allowed to shine through. As a result, gd
 * automatically blends the existing color at that point with the drawing
 * color, and stores the result in the image. The resulting pixel is opaque.
 * In non-blending mode, the drawing color is copied literally with its alpha
 * channel information, replacing the destination pixel. Blending mode is not
 * available when drawing on palette images.
 */
<<__Native>>
function imagealphablending(resource $image,
                            bool $blendmode): bool;

/* Activate the fast drawing antialiased methods for lines and wired polygons.
 * It does not support alpha components. It works using a direct blend
 * operation. It works only with truecolor images.  Thickness and styled are
 * not supported.  Using antialiased primitives with transparent background
 * color can end with some unexpected results. The blend method uses the
 * background color as any other colors. The lack of alpha component support
 * does not allow an alpha based antialiasing method.
 */
<<__Native>>
function imageantialias(resource $image,
                        bool $on): bool;

/* imagearc() draws an arc of circle centered at the given coordinates.
 */
<<__Native>>
function imagearc(resource $image,
                  int $cx,
                  int $cy,
                  int $width,
                  int $height,
                  int $start,
                  int $end,
                  int $color): bool;

/* imagechar() draws the first character of c in the image identified by image
 * with its upper-left at x,y (top left is 0, 0) with the color color.
 */
<<__Native>>
function imagechar(resource $image,
                   int $font,
                   int $x,
                   int $y,
                   string $c,
                   int $color): bool;

/* Draws the character c vertically at the specified coordinate on the given
 * image.
 */
<<__Native>>
function imagecharup(resource $image,
                     int $font,
                     int $x,
                     int $y,
                     string $c,
                     int $color): bool;

/* Returns a color identifier representing the color composed of the given RGB
 * components.  imagecolorallocate() must be called to create each color that
 * is to be used in the image represented by image.  The first call to
 * imagecolorallocate() fills the background color in palette-based images -
 * images created using imagecreate(). These parameters are integers between 0
 * and 255 or hexadecimals between 0x00 and 0xFF.
 */
<<__Native>>
function imagecolorallocate(resource $image,
                            int $red,
                            int $green,
                            int $blue): mixed;

/* imagecolorallocatealpha() behaves identically to imagecolorallocate() with
 * the addition of the transparency parameter alpha. The colors parameters are
 * integers between 0 and 255 or hexadecimals between 0x00 and 0xFF.
 */
<<__Native>>
function imagecolorallocatealpha(resource $image,
                                 int $red,
                                 int $green,
                                 int $blue,
                                 int $alpha): mixed;

/* Returns the index of the color of the pixel at the specified location in
 * the image specified by image.  If PHP is compiled against GD library 2.0 or
 * higher and the image is a truecolor image, this function returns the RGB
 * value of that pixel as integer. Use bitshifting and masking to access the
 * distinct red, green and blue component values:
 */
<<__Native>>
function imagecolorat(resource $image,
                      int $x,
                      int $y): mixed;

/* Returns the index of the color in the palette of the image which is
 * "closest" to the specified RGB value.  The "distance" between the desired
 * color and each color in the palette is calculated as if the RGB values
 * represented points in three-dimensional space.  If you created the image
 * from a file, only colors used in the image are resolved. Colors present
 * only in the palette are not resolved. The colors parameters are integers
 * between 0 and 255 or hexadecimals between 0x00 and 0xFF.
 */
<<__Native>>
function imagecolorclosest(resource $image,
                           int $red,
                           int $green,
                           int $blue): mixed;

/* Returns the index of the color in the palette of the image which is
 * "closest" to the specified RGB value and alpha level. The colors parameters
 * are integers between 0 and 255 or hexadecimals between 0x00 and 0xFF.
 */
<<__Native>>
function imagecolorclosestalpha(resource $image,
                                int $red,
                                int $green,
                                int $blue,
                                int $alpha): mixed;

/* Get the index of the color which has the hue, white and blackness nearest
 * the given color.
 */
<<__Native>>
function imagecolorclosesthwb(resource $image,
                              int $red,
                              int $green,
                              int $blue): mixed;

/* De-allocates a color previously allocated with imagecolorallocate() or
 * imagecolorallocatealpha().
 */
<<__Native>>
function imagecolordeallocate(resource $image,
                              int $color): bool;

/* Returns the index of the specified color in the palette of the image.  If
 * you created the image from a file, only colors used in the image are
 * resolved. Colors present only in the palette are not resolved.
 */
<<__Native>>
function imagecolorexact(resource $image,
                         int $red,
                         int $green,
                         int $blue): mixed;

/* Returns the index of the specified color+alpha in the palette of the image.
 * The colors parameters are integers between 0 and 255 or hexadecimals
 * between 0x00 and 0xFF.
 */
<<__Native>>
function imagecolorexactalpha(resource $image,
                              int $red,
                              int $green,
                              int $blue,
                              int $alpha): mixed;

/* Makes the colors of the palette version of an image more closely match the
 * true color version.
 */
<<__Native>>
function imagecolormatch(resource $image1,
                         resource $image2): mixed;

/* This function is guaranteed to return a color index for a requested color,
 * either the exact color or the closest possible alternative.  If you created
 * the image from a file, only colors used in the image are resolved. Colors
 * present only in the palette are not resolved.
 */
<<__Native>>
function imagecolorresolve(resource $image,
                           int $red,
                           int $green,
                           int $blue): mixed;

/* This function is guaranteed to return a color index for a requested color,
 * either the exact color or the closest possible alternative. The colors
 * parameters are integers between 0 and 255 or hexadecimals between 0x00 and
 * 0xFF.
 */
<<__Native>>
function imagecolorresolvealpha(resource $image,
                                int $red,
                                int $green,
                                int $blue,
                                int $alpha): mixed;

/* This sets the specified index in the palette to the specified color. This
 * is useful for creating flood-fill-like effects in palleted images without
 * the overhead of performing the actual flood-fill.
 */
<<__Native>>
function imagecolorset(resource $image,
                       int $index,
                       int $red,
                       int $green,
                       int $blue): mixed;

/* Gets the color for a specified index.
 */
<<__Native>>
function imagecolorsforindex(resource $image,
                             int $index): mixed;

/* Returns the number of colors in an image palette.
 */
<<__Native>>
function imagecolorstotal(resource $image): mixed;

/* Sets the transparent color in the given image.
 */
<<__Native>>
function imagecolortransparent(resource $image,
                               int $color = -1): mixed;

/* Applies a convolution matrix on the image, using the given coefficient and
 * offset.
 */
<<__Native>>
function imageconvolution(
  resource $image,
  varray<varray<float>> $matrix,
  float $div,
  float $offset,
): bool;

/* Copy a part of src_im onto dst_im starting at the x,y coordinates src_x,
 * src_y with a width of src_w and a height of src_h. The portion defined will
 * be copied onto the x,y coordinates, dst_x and dst_y.
 */
<<__Native>>
function imagecopy(resource $dst_im,
                   resource $src_im,
                   int $dst_x,
                   int $dst_y,
                   int $src_x,
                   int $src_y,
                   int $src_w,
                   int $src_h): bool;

/* Copy a part of src_im onto dst_im starting at the x,y coordinates src_x,
 * src_y with a width of src_w and a height of src_h. The portion defined will
 * be copied onto the x,y coordinates, dst_x and dst_y.
 */
<<__Native>>
function imagecopymerge(resource $dst_im,
                        resource $src_im,
                        int $dst_x,
                        int $dst_y,
                        int $src_x,
                        int $src_y,
                        int $src_w,
                        int $src_h,
                        int $pct): bool;

/* imagecopymergegray() copy a part of src_im onto dst_im starting at the x,y
 * coordinates src_x, src_y with a width of src_w and a height of src_h. The
 * portion defined will be copied onto the x,y coordinates, dst_x and dst_y.
 * This function is identical to imagecopymerge() except that when merging it
 * preserves the hue of the source by converting the destination pixels to
 * gray scale before the copy operation.
 */
<<__Native>>
function imagecopymergegray(resource $dst_im,
                            resource $src_im,
                            int $dst_x,
                            int $dst_y,
                            int $src_x,
                            int $src_y,
                            int $src_w,
                            int $src_h,
                            int $pct): bool;

/* imagecopyresampled() copies a rectangular portion of one image to another
 * image, smoothly interpolating pixel values so that, in particular, reducing
 * the size of an image still retains a great deal of clarity.  In other
 * words, imagecopyresampled() will take an rectangular area from src_image of
 * width src_w and height src_h at position (src_x,src_y) and place it in a
 * rectangular area of dst_image of width dst_w and height dst_h at position
 * (dst_x,dst_y).  If the source and destination coordinates and width and
 * heights differ, appropriate stretching or shrinking of the image fragment
 * will be performed. The coordinates refer to the upper left corner. This
 * function can be used to copy regions within the same image (if dst_image is
 * the same as src_image) but if the regions overlap the results will be
 * unpredictable.
 */
<<__Native>>
function imagecopyresampled(resource $dst_im,
                            resource $src_im,
                            int $dst_x,
                            int $dst_y,
                            int $src_x,
                            int $src_y,
                            int $dst_w,
                            int $dst_h,
                            int $src_w,
                            int $src_h): bool;

/* imagecopyresized() copies a rectangular portion of one image to another
 * image. dst_image is the destination image, src_image is the source image
 * identifier.  In other words, imagecopyresized() will take an rectangular
 * area from src_image of width src_w and height src_h at position
 * (src_x,src_y) and place it in a rectangular area of dst_image of width
 * dst_w and height dst_h at position (dst_x,dst_y).  If the source and
 * destination coordinates and width and heights differ, appropriate
 * stretching or shrinking of the image fragment will be performed. The
 * coordinates refer to the upper left corner. This function can be used to
 * copy regions within the same image (if dst_image is the same as src_image)
 * but if the regions overlap the results will be unpredictable.
 */
<<__Native>>
function imagecopyresized(resource $dst_im,
                          resource $src_im,
                          int $dst_x,
                          int $dst_y,
                          int $src_x,
                          int $src_y,
                          int $dst_w,
                          int $dst_h,
                          int $src_w,
                          int $src_h): bool;

/* imagecreate() returns an image identifier representing a blank image of
 * specified size.  We recommend the use of imagecreatetruecolor().
 */
<<__Native>>
function imagecreate(int $width,
                     int $height): mixed;

/* Create a new image from a given part of GD2 file or URL. TipA URL can be
 * used as a filename with this function if the fopen wrappers have been
 * enabled. See fopen() for more details on how to specify the filename. See
 * the List of Supported Protocols/Wrappers for links to information about
 * what abilities the various wrappers have, notes on their usage, and
 * information on any predefined variables they may provide.
 */
<<__Native>>
function imagecreatefromgd2part(string $filename,
                                int $srcx,
                                int $srcy,
                                int $width,
                                int $height): mixed;

/* Create a new image from GD file or URL. TipA URL can be used as a filename
 * with this function if the fopen wrappers have been enabled. See fopen() for
 * more details on how to specify the filename. See the List of Supported
 * Protocols/Wrappers for links to information about what abilities the
 * various wrappers have, notes on their usage, and information on any
 * predefined variables they may provide.
 */
<<__Native>>
function imagecreatefromgd(string $filename): mixed;

/* Create a new image from GD2 file or URL. TipA URL can be used as a filename
 * with this function if the fopen wrappers have been enabled. See fopen() for
 * more details on how to specify the filename. See the List of Supported
 * Protocols/Wrappers for links to information about what abilities the
 * various wrappers have, notes on their usage, and information on any
 * predefined variables they may provide.
 */
<<__Native>>
function imagecreatefromgd2(string $filename): mixed;

/* imagecreatefromgif() returns an image identifier representing the image
 * obtained from the given filename. TipA URL can be used as a filename with
 * this function if the fopen wrappers have been enabled. See fopen() for more
 * details on how to specify the filename. See the List of Supported
 * Protocols/Wrappers for links to information about what abilities the
 * various wrappers have, notes on their usage, and information on any
 * predefined variables they may provide.
 */
<<__Native>>
function imagecreatefromgif(string $filename): mixed;

/* imagecreatefromjpeg() returns an image identifier representing the image
 * obtained from the given filename. TipA URL can be used as a filename with
 * this function if the fopen wrappers have been enabled. See fopen() for more
 * details on how to specify the filename. See the List of Supported
 * Protocols/Wrappers for links to information about what abilities the
 * various wrappers have, notes on their usage, and information on any
 * predefined variables they may provide.
 */
<<__Native>>
function imagecreatefromjpeg(string $filename): mixed;

/* imagecreatefrompng() returns an image identifier representing the image
 * obtained from the given filename. TipA URL can be used as a filename with
 * this function if the fopen wrappers have been enabled. See fopen() for more
 * details on how to specify the filename. See the List of Supported
 * Protocols/Wrappers for links to information about what abilities the
 * various wrappers have, notes on their usage, and information on any
 * predefined variables they may provide.
 */
<<__Native>>
function imagecreatefrompng(string $filename): mixed;

/* imagecreatefromstring() returns an image identifier representing the image
 * obtained from the given data. These types will be automatically detected if
 * your build of PHP supports them: JPEG, PNG, GIF, WBMP, and GD2.
 */
<<__Native>>
function imagecreatefromstring(string $data): mixed;

/* imagecreatefromwbmp() returns an image identifier representing the image
 * obtained from the given filename. TipA URL can be used as a filename with
 * this function if the fopen wrappers have been enabled. See fopen() for more
 * details on how to specify the filename. See the List of Supported
 * Protocols/Wrappers for links to information about what abilities the
 * various wrappers have, notes on their usage, and information on any
 * predefined variables they may provide.
 */
<<__Native>>
function imagecreatefromwbmp(string $filename): mixed;

/* imagecreatefromxbm() returns an image identifier representing the image
 * obtained from the given filename. TipA URL can be used as a filename with
 * this function if the fopen wrappers have been enabled. See fopen() for more
 * details on how to specify the filename. See the List of Supported
 * Protocols/Wrappers for links to information about what abilities the
 * various wrappers have, notes on their usage, and information on any
 * predefined variables they may provide.
 */
<<__Native>>
function imagecreatefromxbm(string $filename): mixed;

/* imagecreatetruecolor() returns an image identifier representing a black
 * image of the specified size.  Depending on your PHP and GD versions this
 * function is defined or not. With PHP 4.0.6 through 4.1.x this function
 * always exists if the GD module is loaded, but calling it without GD2 being
 * installed PHP will issue a fatal error and exit. With PHP 4.2.x this
 * behaviour is different in issuing a warning instead of an error. Other
 * versions only define this function, if the correct GD version is installed.
 */
<<__Native>>
function imagecreatetruecolor(int $width,
                              int $height): mixed;

/* Crop an image using the given coordinates and size, x, y, width and height.
 */
<<__Native>>
function imagecrop(
  resource $image,
  shape(
    'x' => int,
    'y' => int,
    'width' => int,
    'height' => int,
  ) $rect,
): mixed;

/* Crop an image automatically using one of the available modes.
 */
<<__Native>>
function imagecropauto(resource $image, int $mode = -1,
                       float $threshold = 0.5, int $color = -1): mixed;

/* This function is deprecated. Use combination of imagesetstyle() and
 * imageline() instead.
 */
<<__Native>>
function imagedashedline(resource $image,
                         int $x1,
                         int $y1,
                         int $x2,
                         int $y2,
                         int $color): bool;

/* imagedestroy() frees any memory associated with image image.
 */
<<__Native>>
function imagedestroy(resource $image): bool;

/* Draws an ellipse centered at the specified coordinates.
 */
<<__Native>>
function imageellipse(resource $image,
                      int $cx,
                      int $cy,
                      int $width,
                      int $height,
                      int $color): bool;

/* Draws an ellipse centered at the specified coordinates.
 */
<<__Native>>
function imageflip(resource $image, int $mode = -1): bool;

/* Performs a flood fill starting at the given coordinate (top left is 0, 0)
 * with the given color in the image.
 */
<<__Native>>
function imagefill(resource $image,
                   int $x,
                   int $y,
                   int $color): bool;

/* Draws a partial arc centered at the specified coordinate in the given
 * image.
 */
<<__Native>>
function imagefilledarc(resource $image,
                        int $cx,
                        int $cy,
                        int $width,
                        int $height,
                        int $start,
                        int $end,
                        int $color,
                        int $style): bool;

/* Draws an ellipse centered at the specified coordinate on the given image.
 */
<<__Native>>
function imagefilledellipse(resource $image,
                            int $cx,
                            int $cy,
                            int $width,
                            int $height,
                            int $color): bool;

/* imagefilledpolygon() creates a filled polygon in the given image.
 */
<<__Native>>
function imagefilledpolygon(resource $image,
                            varray<int> $points,
                            int $num_points,
                            int $color): bool;

/* Creates a rectangle filled with color in the given image starting at point
 * 1 and ending at point 2. 0, 0 is the top left corner of the image.
 */
<<__Native>>
function imagefilledrectangle(resource $image,
                              int $x1,
                              int $y1,
                              int $x2,
                              int $y2,
                              int $color): bool;

/* imagefilltoborder() performs a flood fill whose border color is defined by
 * border. The starting point for the fill is x, y (top left is 0, 0) and the
 * region is filled with color color.
 */
<<__Native>>
function imagefilltoborder(resource $image,
                           int $x,
                           int $y,
                           int $border,
                           int $color): bool;

/* imagefilter() applies the given filter filtertype on the image.
 */
<<__Native>>
function imagefilter(resource $image,
                     int $filtertype,
                     mixed $arg1 = 0,
                     mixed $arg2 = 0,
                     mixed $arg3 = 0,
                     mixed $arg4 = 0): bool;

/* Returns the pixel height of a character in the specified font.
 */
<<__Native>>
function imagefontheight(int $font): int;

/* Returns the pixel width of a character in font.
 */
<<__Native>>
function imagefontwidth(int $font): int;

/* This function calculates and returns the bounding box in pixels for a
 * FreeType text.
 */
<<__Native>>
function imageftbbox(float $size,
                     float $angle,
                     string $font_file,
                     string $text,
                     shape(?'linespacing' => float) $extrainfo = shape()): mixed;

<<__Native>>
function imagefttext(resource $image,
                     mixed $size,
                     mixed $angle,
                     int $x,
                     int $y,
                     int $col,
                     string $font_file,
                     string $text,
                     shape(?'linespacing' => float) $extrainfo = shape()): mixed;

/* Applies gamma correction to the given gd image given an input and an output
 * gamma.
 */
<<__Native>>
function imagegammacorrect(resource $image,
                           float $inputgamma,
                           float $outputgamma): bool;

/* Outputs a GD2 image to the given filename.
 */
<<__Native>>
function imagegd2(resource $image,
                  string $filename = "",
                  int $chunk_size = 0,
                  int $type = 0): bool;

/* Outputs a GD image to the given filename.
 */
<<__Native>>
function imagegd(resource $image,
                 string $filename = ""): bool;

/* imagegif() creates the GIF file in filename from the image image. The image
 * argument is the return from the imagecreate() or imagecreatefrom* function.
 *  The image format will be GIF87a unless the image has been made transparent
 * with imagecolortransparent(), in which case the image format will be
 * GIF89a.
 */
<<__Native>>
function imagegif(resource $image,
                  string $filename = ""): bool;

/* imageinterlace() turns the interlace bit on or off.  If the interlace bit
 * is set and the image is used as a JPEG image, the image is created as a
 * progressive JPEG.
 */
<<__Native>>
function imageinterlace(resource $image,
                        ?int $interlace = null): mixed;

/* imageistruecolor() finds whether the image image is a truecolor image.
 */
<<__Native>>
function imageistruecolor(resource $image): bool;

/* imagejpeg() creates a JPEG file from the given image.
 */
<<__Native>>
function imagejpeg(resource $image,
                   string $filename = "",
                   int $quality = -1): bool;

/* Set the alpha blending flag to use the bundled libgd layering effects.
 */
<<__Native>>
function imagelayereffect(resource $image,
                          int $effect): bool;

/* Draws a line between the two given points.
 */
<<__Native>>
function imageline(resource $image,
                   int $x1,
                   int $y1,
                   int $x2,
                   int $y2,
                   int $color): bool;

/* imageloadfont() loads a user-defined bitmap and returns its identifier.
 */
<<__Native>>
function imageloadfont(string $file): mixed;

/* Outputs or saves a PNG image from the given image.
 */
<<__Native>>
function imagepng(resource $image,
                  string $filename = "",
                  int $quality = -1,
                  int $filters = -1): bool;

/* imagepolygon() creates a polygon in the given image.
 */
<<__Native>>
function imagepolygon(resource $image,
                      varray<int> $points,
                      int $num_points,
                      int $color): bool;

/* imagerectangle() creates a rectangle starting at the specified coordinates.
 */
<<__Native>>
function imagerectangle(resource $image,
                        int $x1,
                        int $y1,
                        int $x2,
                        int $y2,
                        int $color): bool;

/* Rotates the image image using the given angle in degrees.  The center of
 * rotation is the center of the image, and the rotated image may have
 * different dimensions than the original image.
 */
<<__Native>>
function imagerotate(resource $source_image,
                     float $angle,
                     int $bgd_color,
                     int $ignore_transparent = 0): mixed;

/* imagesavealpha() sets the flag to attempt to save full alpha channel
 * information (as opposed to single-color transparency) when saving PNG
 * images.  You have to unset alphablending (imagealphablending($im, false)),
 * to use it.  Alpha channel is not supported by all browsers, if you have
 * problem with your browser, try to load your script with an alpha channel
 * compliant browser, e.g. latest Mozilla.
 */
<<__Native>>
function imagesavealpha(resource $image,
                        bool $saveflag): bool;

/*
 * imagescale - Scale an image using the given new width and height.
 */
<<__Native>>
function imagescale(resource $img, int $newwidth, int $newheigh = -1,
                               int $method = IMG_BILINEAR_FIXED): mixed;

/* imagesetbrush() sets the brush image to be used by all line drawing
 * functions (such as imageline() and imagepolygon()) when drawing with the
 * special colors IMG_COLOR_BRUSHED or IMG_COLOR_STYLEDBRUSHED.
 */
<<__Native>>
function imagesetbrush(resource $image,
                       resource $brush): bool;

/* imagesetpixel() draws a pixel at the specified coordinate.
 */
<<__Native>>
function imagesetpixel(resource $image,
                       int $x,
                       int $y,
                       int $color): bool;

/* imagesetstyle() sets the style to be used by all line drawing functions
 * (such as imageline() and imagepolygon()) when drawing with the special
 * color IMG_COLOR_STYLED or lines of images with color
 * IMG_COLOR_STYLEDBRUSHED.
 */
<<__Native>>
function imagesetstyle(resource $image,
                       varray<int> $style): bool;

/* imagesetthickness() sets the thickness of the lines drawn when drawing
 * rectangles, polygons, ellipses etc. etc. to thickness pixels.
 */
<<__Native>>
function imagesetthickness(resource $image,
                           int $thickness): bool;

/* imagesettile() sets the tile image to be used by all region filling
 * functions (such as imagefill() and imagefilledpolygon()) when filling with
 * the special color IMG_COLOR_TILED.  A tile is an image used to fill an area
 * with a repeated pattern. Any GD image can be used as a tile, and by setting
 * the transparent color index of the tile image with imagecolortransparent(),
 * a tile allows certain parts of the underlying area to shine through can be
 * created.  You need not take special action when you are finished with a
 * tile, but if you destroy the tile image, you must not use the
 * IMG_COLOR_TILED color until you have set a new tile image!
 */
<<__Native>>
function imagesettile(resource $image,
                      resource $tile): bool;

/* Draws a string at the given coordinates.
 */
<<__Native>>
function imagestring(resource $image,
                     int $font,
                     int $x,
                     int $y,
                     string $str,
                     int $color): bool;

/* Draws a string vertically at the given coordinates.
 */
<<__Native>>
function imagestringup(resource $image,
                       int $font,
                       int $x,
                       int $y,
                       string $str,
                       int $color): bool;

/* Returns the width of the given image resource.
 */
<<__Native>>
function imagesx(resource $image): mixed;

/* Returns the height of the given image resource.
 */
<<__Native>>
function imagesy(resource $image): mixed;

/* imagetruecolortopalette() converts a truecolor image to a palette image.
 * The code for this function was originally drawn from the Independent JPEG
 * Group library code, which is excellent. The code has been modified to
 * preserve as much alpha channel information as possible in the resulting
 * palette, in addition to preserving colors as well as possible. This does
 * not work as well as might be hoped. It is usually best to simply produce a
 * truecolor output image instead, which guarantees the highest output
 * quality.
 */
<<__Native>>
function imagetruecolortopalette(resource $image,
                                 bool $dither,
                                 int $ncolors): mixed;

/* This function calculates and returns the bounding box in pixels for a
 * TrueType text.
 */
<<__Native>>
function imagettfbbox(float $size,
                      float $angle,
                      string $fontfile,
                      string $text): mixed;

/* Writes the given text into the image using TrueType fonts.
 */
<<__Native>>
function imagettftext(resource $image,
                      mixed $size,
                      mixed $angle,
                      int $x,
                      int $y,
                      int $color,
                      string $fontfile,
                      string $text): mixed;

/* Returns the image types supported by the current PHP installation.
 */
<<__Native>>
function imagetypes(): int;

/* imagewbmp() outputs or save a WBMP version of the given image.
 */
<<__Native>>
function imagewbmp(resource $image,
                   string $filename = "",
                   int $foreground = -1): bool;

/* Embeds binary IPTC data into a JPEG image.
 */
<<__Native>>
function iptcembed(string $iptcdata,
                   string $jpeg_file_name,
                   int $spool = 0): mixed;

<<__Native>>
function iptcparse(string $iptcblock): mixed;

/* Converts a JPEG file into a WBMP file.
 */
<<__Native>>
function jpeg2wbmp(string $jpegname,
                   string $wbmpname,
                   int $dest_height,
                   int $dest_width,
                   int $threshold): bool;

/* Converts a PNG file into a WBMP file.
 */
<<__Native>>
function png2wbmp(string $pngname,
                  string $wbmpname,
                  int $dest_height,
                  int $dest_width,
                  int $threshold): bool;

/**
 * imagepalettecopy() copies the palette from the source image
 * to the destination image.
 */
<<__Native>>
function imagepalettecopy(resource $dst,
                          resource $src): mixed;

/**
 * Sets the interpolation method, setting an interpolation method
 * effects the rendering of various functions in GD,
 * such as the imagerotate() function.
 */
<<__Native>>
function imagesetinterpolation(resource $img,
                               int $method = IMG_BILINEAR_FIXED): bool;
