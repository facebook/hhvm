<?hh
<<__EntryPoint>> function main(): void {

    echo "PNG compression test: ";

    $im = imagecreatetruecolor(20,20);
    imagefilledrectangle($im, 5,5, 10,10, 0xffffff);
    imagepng($im, __SystemLib\hphp_test_tmppath('test_pngcomp.png'), 9);

    $im2 = imagecreatefrompng(__SystemLib\hphp_test_tmppath('test_pngcomp.png'));
    $col = imagecolorat($im2, 8,8);
    if ($col == 0xffffff) {
            echo "ok\n";
    }

    unlink(__SystemLib\hphp_test_tmppath('test_pngcomp.png'));
}
