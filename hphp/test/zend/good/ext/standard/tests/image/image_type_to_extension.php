<?hh
<<__EntryPoint>> function main(): void {
    $constants = dict[
        "IMAGETYPE_GIF"      => IMAGETYPE_GIF,
        "IMAGETYPE_JPEG"     => IMAGETYPE_JPEG,
        "IMAGETYPE_PNG"      => IMAGETYPE_PNG,
        "IMAGETYPE_SWF"      => IMAGETYPE_SWF,
        "IMAGETYPE_PSD"      => IMAGETYPE_PSD,
        "IMAGETYPE_BMP"      => IMAGETYPE_BMP,
        "IMAGETYPE_TIFF_II"  => IMAGETYPE_TIFF_II,
        "IMAGETYPE_TIFF_MM"  => IMAGETYPE_TIFF_MM,
        "IMAGETYPE_JPC"      => IMAGETYPE_JPC,
        "IMAGETYPE_JP2"      => IMAGETYPE_JP2,
        "IMAGETYPE_JPX"      => IMAGETYPE_JPX,
        "IMAGETYPE_JB2"      => IMAGETYPE_JB2,
        "IMAGETYPE_IFF"      => IMAGETYPE_IFF,
        "IMAGETYPE_WBMP"     => IMAGETYPE_WBMP,
        "IMAGETYPE_WEBP"     => IMAGETYPE_WEBP,
        "IMAGETYPE_JPEG2000" => IMAGETYPE_JPEG2000,
        "IMAGETYPE_XBM"      => IMAGETYPE_XBM
    ];
    foreach($constants as $name => $constant) {
        printf("Constant: %s\n\tWith dot: %s\n\tWithout dot: %s\n", $name, image_type_to_extension($constant), image_type_to_extension($constant, false));
    }
    var_dump(image_type_to_extension(1000000, false));
    try { var_dump(image_type_to_extension()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
    var_dump(image_type_to_extension(0));
    try { var_dump(image_type_to_extension(0, 0, 0)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "Done";
}
