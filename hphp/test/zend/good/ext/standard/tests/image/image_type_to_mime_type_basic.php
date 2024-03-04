<?hh

/* Prototype  : string image_type_to_mime_type  ( int $imagetype  )
 * Description: Get Mime-Type for image-type returned by getimagesize, exif_read_data, exif_thumbnail, exif_imagetype.
 * Source code: ext/standard/image.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "Starting image_type_to_mime_type() test\n\n";

$image_types = vec[
    IMAGETYPE_GIF,
    IMAGETYPE_JPEG,
    IMAGETYPE_PNG,
    IMAGETYPE_SWF,
    IMAGETYPE_PSD,
    IMAGETYPE_BMP,
    IMAGETYPE_TIFF_II,
    IMAGETYPE_TIFF_MM,
    IMAGETYPE_JPC,
    IMAGETYPE_JP2,
    IMAGETYPE_JPX,
    IMAGETYPE_JB2,
    IMAGETYPE_IFF,
    IMAGETYPE_WBMP,
    IMAGETYPE_WEBP,
    IMAGETYPE_JPEG2000,
    IMAGETYPE_AVIF,
    IMAGETYPE_HEIC,
    IMAGETYPE_XBM
];

    foreach($image_types as $image_type) {
        var_dump(image_type_to_mime_type($image_type));
    }

echo "\nDone image_type_to_mime_type() test\n";
}
