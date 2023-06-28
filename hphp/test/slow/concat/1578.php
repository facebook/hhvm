<?hh


<<__EntryPoint>>
function main_1578() :mixed{
$v = 1;
echo $v . b'a' . b"b" . shell_exec("ls \055\144 \x2ftmp");
echo b'a' . b"b" . shell_exec("ls \055\144 \x2ftmp") . $v;
}
