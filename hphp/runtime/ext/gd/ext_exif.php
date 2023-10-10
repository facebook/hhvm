<?hh

/* exif_imagetype() reads the first bytes of an image and checks its
 * signature.  exif_imagetype() can be used to avoid calls to other exif
 * functions with unsupported file types or in conjunction with
 * $_SERVER['HTTP_ACCEPT'] to check whether or not the viewer is able to see a
 * specific image in the browser.
 */
<<__Native>>
function exif_imagetype(string $filename): mixed;

<<__Native>>
function exif_read_data(string $filename,
                        string $sections = "",
                        bool $arrays = false,
                        bool $thumbnail = false): mixed;

function read_exif_data(string $filename,
                        string $sections = "",
                        bool $arrays = false,
                        bool $thumbnail = false): mixed {
  return exif_read_data($filename, $sections, $arrays, $thumbnail);
}

<<__Native>>
function exif_tagname(int $index): mixed;

/* exif_thumbnail() reads the embedded thumbnail of a TIFF or JPEG image.  If
 * you want to deliver thumbnails through this function, you should send the
 * mimetype information using the header() function.  It is possible that
 * exif_thumbnail() cannot create an image but can determine its size. In this
 * case, the return value is FALSE but width and height are set.
 */
<<__Native>>
function exif_thumbnail(string $filename,
                        <<__OutOnly("KindOfInt64")>>
                        inout mixed $width,
                        <<__OutOnly("KindOfInt64")>>
                        inout mixed $height,
                        <<__OutOnly("KindOfInt64")>>
                        inout mixed $imagetype): mixed;
