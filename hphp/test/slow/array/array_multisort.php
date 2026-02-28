<?hh


<<__EntryPoint>>
function main_array_multisort() :mixed{
  $asc = SORT_ASC;
  $desc = SORT_DESC;
  $natural = SORT_NATURAL;
  $a = vec['img99.jpg', 'img1.jpg', 'img2.jpg', 'img12.jpg', 'img10.jpg'];
  array_multisort1(inout $a);
  var_dump($a);
  array_multisort3(inout $a, inout $asc, inout $natural);
  var_dump($a);
  array_multisort3(inout $a, inout $desc, inout $natural);
  var_dump($a);
}
