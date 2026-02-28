<?hh <<__EntryPoint>> function main(): void {
ini_set('bcmath.scale', 5);
echo bcadd("2.2", "4.3", 2)."\n";
echo bcadd("2.2", "-7.3", 1)."\n";
echo bcadd("-4.27", "7.3");
}
