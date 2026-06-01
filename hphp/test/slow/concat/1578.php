<?hh


<<__EntryPoint>>
function main_1578() :mixed{
$v = 1;
echo $v . 'a' . "b" . shell_exec("ls \055\144 \x2ftmp");
echo 'a' . "b" . shell_exec("ls \055\144 \x2ftmp") . $v;
}
