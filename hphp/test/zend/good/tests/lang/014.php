<?hh
function F ($a) {
    eval('function FImpl() { '.$a.' }');
    FImpl();
}
<<__EntryPoint>> function main(): void {
// error_reporting(0);
F("echo \"Hello\";");
}
