<?hh

<<__EntryPoint>>
function main_preg_filter() :mixed{
$pattern = varray['/\d/', '/[a-z]/', '/[1a]/'];
$replace = varray['A:$0', 'B:$0', 'C:$0'];
$subject = varray['1', 'a', '2', 'b', '3', 'A', 'B', '4'];
$limit = -1;
$count = -1;

var_dump(preg_filter($pattern, $replace, $subject, -1, inout $count));
var_dump(preg_filter($pattern, $replace, $subject, $limit, inout $count));
var_dump(preg_filter($pattern, $replace, $subject, $limit, inout $count));
var_dump($count);

$subject = '1024';
$count = -1;

var_dump(preg_filter($pattern, $replace, $subject, $limit, inout $count));
var_dump($count);

$subject = 'XYZ';
$count = -1;

var_dump(preg_filter($pattern, $replace, $subject, $limit, inout $count));
var_dump($count);

$subject = varray['0', '00', '000', '0000', '00000'];
$limit = 3;
$count = -1;

var_dump(preg_filter($pattern, $replace, $subject, $limit, inout $count));
var_dump($count);
}
