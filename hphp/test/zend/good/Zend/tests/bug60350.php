<?hh <<__EntryPoint>> function main() {
$str = "\e";
if (ord($str) == 27) {
    echo "Works";
}
}
