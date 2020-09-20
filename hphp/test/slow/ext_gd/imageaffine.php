<?hh


<<__EntryPoint>>
function main_imageaffine() {
$base_img = tempnam('/tmp', 'test-imageaffine');
$tgt_img = tempnam('/tmp', 'test-imageaffine');

$toDelete = varray[$base_img, $tgt_img];

$arrAffine = varray[
    varray[ 1, 0, 0, 1, 0, 0 ],
    varray[ 1, 0, 0, 1, 150, 0 ],
    varray[ 1.2, 0, 0, 0.6, 0, 0 ],
    varray[ -1.2, 0, 0, -0.6, 0, 0 ],
    varray[ 1, 2, 0, 1, 0, 0 ],
    varray[ 2, 1, 0, 1, 0, 0 ],
    varray[ cos(15.0), sin(15.0), -sin(15.0), cos(15.0), 0, 0 ],
    varray[ cos(15.0), -sin(15.0), sin(15.0), cos(15.0), 0, 0 ]
];

$RSR_base = imagecreatetruecolor(400, 300);
$w = imagesx($RSR_base);
$h = imagesy($RSR_base);
$arrClip = darray[ 'x' => 0, 'y' => 0, 'width' => $w, 'height' => $h ];
$fillcolor = imagecolorallocate($RSR_base, 0, 0, 0);
imagefill($RSR_base, 10, 10, $fillcolor);
imagepng($RSR_base, $base_img);
var_dump(md5_file($base_img));
$drawcolor = imagecolorallocate($RSR_base, 255, 0, 0);
$triangle = varray[ 50, 50, 50, 150, 200, 150 ];
$points = 3;
imageantialias($RSR_base, true);
$drawtriangle = imagefilledpolygon($RSR_base, $triangle,
                                    $points, $drawcolor);
imagepng($RSR_base, $tgt_img);
var_dump(md5_file($tgt_img));
foreach($arrAffine as $aff) {
    $dst = tempnam('/tmp', 'test-imageaffine');
    $toDelete[] = $dst;
    $RSRaff2 = imageaffine($RSR_base, $aff, $arrClip);
    imagepng($RSRaff2, $dst, 9);
    var_dump(md5_file($dst));
}
foreach ($toDelete as $file) {
    @unlink($file);
}
}
