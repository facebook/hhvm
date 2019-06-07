<?hh
interface I {
    const FOO = 10;
}

class C implements I {
    const FOO = 10;
}
<<__EntryPoint>> function main() {
echo "Done\n";
}
