<?hh
function F ($a) {
    eval($a);
}
<<__EntryPoint>> function main(): void {
error_reporting(0);
F("echo \"Hello\";");
}
