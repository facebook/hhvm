<?hh

<<__EntryPoint>>
function main_bug_13346458() {
$img = imagecreatetruecolor(1, 1);
imagesetstyle($img, varray[]);
imagesetpixel($img, 0, 0, -2);
imagedestroy($img);
echo "Done\n";
}
