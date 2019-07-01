<?hh
function something() {
        foreach(array(1, 2) as $value) {
                for($i = 0; $i < 1; $i++) {
                        continue 2;
                }
                return;
        }
}
<<__EntryPoint>> function main(): void {
something();
echo "ok\n";
}
