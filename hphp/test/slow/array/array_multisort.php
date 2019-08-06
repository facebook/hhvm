<?hh


<<__EntryPoint>>
function main_array_multisort() {
  $asc = SORT_ASC;
  $desc = SORT_DESC;
  $natural = SORT_NATURAL;
  $a = ['img99.jpg', 'img1.jpg', 'img2.jpg', 'img12.jpg', 'img10.jpg'];
  array_multisort1(&$a);
  var_dump($a);
  array_multisort3(&$a, &$asc, &$natural);
  var_dump($a);
  array_multisort3(&$a, &$desc, &$natural);
  var_dump($a);
}
