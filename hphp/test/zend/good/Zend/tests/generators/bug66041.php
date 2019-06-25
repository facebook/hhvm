<?hh
function dumpElement() {
    list($value) = yield;
    var_dump($value);
};
<<__EntryPoint>> function main(): void {
$fixedArray = new SplFixedArray(1);
$fixedArray[0] = 'the element';

$generator = dumpElement();
$generator->next();
$generator->send($fixedArray);
}
