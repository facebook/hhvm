<?hh <<__EntryPoint>> function main(): void {
$str = "\e";
if (ord($str) == 27) {
    echo "Works";
}
}
