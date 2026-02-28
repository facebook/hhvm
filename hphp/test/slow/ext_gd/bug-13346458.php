<?hh

<<__EntryPoint>>
function main_bug_13346458() :mixed{
$img = imagecreatetruecolor(1, 1);
imagesetstyle($img, vec[]);
imagesetpixel($img, 0, 0, -2);
imagedestroy($img);
echo "Done\n";
}
