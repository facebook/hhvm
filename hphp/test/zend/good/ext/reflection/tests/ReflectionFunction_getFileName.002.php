<?hh

/**
 * my doc comment
 */
function foo () :mixed{
    $d = 5;
}

/***
 * not a doc comment
 */
function bar () :mixed{}


function dumpFuncInfo($name) :mixed{
    $funcInfo = new ReflectionFunction($name);
    var_dump($funcInfo->getFileName());
}
<<__EntryPoint>> function main(): void {
dumpFuncInfo('foo');
dumpFuncInfo('bar');
dumpFuncInfo('array_pop');
}
