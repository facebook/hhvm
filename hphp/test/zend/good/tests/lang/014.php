<?hh
function F ($a) {
    eval($a);
}
<<__EntryPoint>> function main() {
error_reporting(0);
F("echo \"Hello\";");
}
