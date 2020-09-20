<?hh


<<__EntryPoint>>
function main_draw_affine() {
$affine_matrix = darray[
  'sx' => 1,
  'rx' => 0,
  'ry' => 0,
  'sy' => 1,
  'tx' => 0,
  'ty' => 0
];

$draw = new ImagickDraw;

$draw->affine($affine_matrix);
echo "PASS\n";

unset($affine_matrix['tx']);
try {
  $draw->affine($affine_matrix);
} catch (Exception $ex) {
  echo "PASS\n";
}

try {
  $draw->affine(darray[]);
} catch (Exception $ex) {
  echo "PASS\n";
}
}
