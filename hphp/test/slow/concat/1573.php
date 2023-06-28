<?hh


<<__EntryPoint>>
function main_1573() :mixed{
echo "a" . "b" . "c" . "d" . "e";
echo 'a' . 'b' . 'c' . 'd' . 'e';
echo 'a' . "b" . "c" . "d" . 'e';
echo '"a' . "\"b" . "\'c" . "\'d" . '\"e';
echo 1 . 2 . 3 . 4 . 5;
echo 1 . '2' . '3' . 4 . 5;
echo 1 . "2" . "3" . 4 . 5;
}
