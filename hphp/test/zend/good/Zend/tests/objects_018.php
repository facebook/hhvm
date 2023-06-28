<?hh

interface Itest {
    function a():mixed;
}

interface Itest2 {
    function a():mixed;
}

interface Itest3 extends Itest, Itest2 {
}
<<__EntryPoint>> function main(): void {
echo "done!\n";
}
