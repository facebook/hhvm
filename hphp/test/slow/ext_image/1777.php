<?hh


<<__EntryPoint>>
function main_1777() :mixed{
$data = 'iVBORw0KGgoAAAANSUhEUgAAABwAAAASCAMAAAB/2U7WAAAABl'       . 'BMVEUAAAD///+l2Z/dAAAASUlEQVR4XqWQUQoAIAxC2/0vXZDr'       . 'EX4IJTRkb7lobNUStXsB0jIXIAMSsQnWlsV+wULF4Avk9fLq2r'       . '8a5HSE35Q3eO2XP1A1wQkZSgETvDtKdQAAAABJRU5ErkJggg==';
$data = base64_decode($data);
$im = imagecreatefromstring($data);
if ($im !== false) {
  ob_start();
  imagepng($im);
  $md5 = md5(ob_get_clean());
  imagedestroy($im);
  echo "md5: $md5\n";
}
else {
    echo 'An error occurred.';
}
}
