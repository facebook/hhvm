<?hh
class Cls {
    function __call($name, $arg) {
    }
}
<<__EntryPoint>> function main() {
$cls = new Cls;
$cls->{0}();
$cls->{1.0}();
$cls->{true}();
$cls->{false}();
$cls->{null}();
}
