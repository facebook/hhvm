<?hh
interface I1 {
    const FOO = 10;
}

interface I2 {
    const FOO = 10;
}

class C implements I1,I2 {
}
<<__EntryPoint>> function main(): void {
echo "Done\n";
}
