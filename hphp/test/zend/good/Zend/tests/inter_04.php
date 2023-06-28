<?hh

interface a {
    function b():mixed;
}

interface b {
    function b():mixed;
}

interface c extends a, b {
}
<<__EntryPoint>> function main(): void {
echo "done!\n";
}
