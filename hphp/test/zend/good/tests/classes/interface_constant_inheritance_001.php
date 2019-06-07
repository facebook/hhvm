<?hh
interface I1 {
    const FOO = 10;
}

interface I2 extends I1 {
    const FOO = 10;
}
<<__EntryPoint>> function main() {
echo "Done\n";
}
