<?hh <<__EntryPoint>> function main(): void {
$no_arg_filters = varray[
    "IMG_FILTER_NEGATE",
    "IMG_FILTER_GRAYSCALE",
    "IMG_FILTER_EDGEDETECT",
    "IMG_FILTER_GAUSSIAN_BLUR",
    "IMG_FILTER_SELECTIVE_BLUR",
    "IMG_FILTER_EMBOSS",
    "IMG_FILTER_MEAN_REMOVAL"
];

$SAVE_DIR = sys_get_temp_dir();
$SOURCE_IMG = __DIR__ . "/test.png";

    foreach ($no_arg_filters as $filt) {
        $im = imagecreatefrompng($SOURCE_IMG);
        if (imagefilter($im, constant($filt))) {
            imagepng($im, $SAVE_DIR."/".$filt. ".png");
            echo "$filt success\n";
            unlink($SAVE_DIR."/".$filt. ".png");
        } else {
            echo "$filt failed\n";
        }
    }

    $im = imagecreatefrompng($SOURCE_IMG);

    if (imagefilter($im, IMG_FILTER_SMOOTH, -1924.124)) {
        imagepng($im, $SAVE_DIR . "/IMG_FILTER_SMOOTH.png");
        echo "IMG_FILTER_SMOOTH success\n";
        unlink($SAVE_DIR . "/IMG_FILTER_SMOOTH.png");
    } else {
        echo "IMG_FILTER_SMOOTH failed\n";
    }

    $im = imagecreatefrompng($SOURCE_IMG);

    if (imagefilter($im, IMG_FILTER_COLORIZE, -127.12, -127.98, 127)) {
        imagepng($im, $SAVE_DIR . "/IMG_FILTER_COLORIZE.png");
        echo "IMG_FILTER_COLORIZE success\n";
        unlink($SAVE_DIR . "/IMG_FILTER_COLORIZE.png");
    } else {
        echo "IMG_FILTER_COLORIZE failed\n";
    }

    $im = imagecreatefrompng($SOURCE_IMG);

    if (imagefilter($im, IMG_FILTER_CONTRAST, -90)) {
        imagepng($im, $SAVE_DIR . "/IMG_FILTER_CONTRAST.png");
        echo "IMG_FILTER_CONTRAST success\n";
        unlink($SAVE_DIR . "/IMG_FILTER_CONTRAST.png");
    } else {
        echo "IMG_FILTER_CONTRAST failed\n";
    }

    $im = imagecreatefrompng($SOURCE_IMG);

    if (imagefilter($im, IMG_FILTER_BRIGHTNESS, 98)) {
        imagepng($im, $SAVE_DIR . "/IMG_FILTER_BRIGHTNESS.png");
        echo "IMG_FILTER_BRIGHTNESS success\n";
        unlink($SAVE_DIR . "/IMG_FILTER_BRIGHTNESS.png");
    } else {
        echo "IMG_FILTER_BRIGHTNESS failed\n";
    }

    $im = imagecreatefrompng($SOURCE_IMG);

    if (imagefilter($im, IMG_FILTER_PIXELATE, 5, true)) {
        imagepng($im, $SAVE_DIR . "/IMG_FILTER_PIXELATE.png");
        echo "IMG_FILTER_PIXELATE success\n";
        unlink($SAVE_DIR . "/IMG_FILTER_PIXELATE.png");
    } else {
        echo "IMG_FILTER_PIXELATE failed\n";
    }
}
