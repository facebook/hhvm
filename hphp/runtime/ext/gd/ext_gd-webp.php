<?hh
/* Outputs or saves a webp image from the given image.
 */
<<__Native>>
function imagewebp(resource $image,
                  string $filename = "",
                  int $quality = 80): bool;

/* Creates a image from the given WebP-file
 */
<<__Native>>
function imagecreatefromwebp(string $filename): mixed;
