<?hh

<<__EntryPoint>>
function main() {
    var_dump(HH\sequence("hello"));
    var_dump(HH\sequence(var_dump("zoink"),2,3));
    var_dump(HH\sequence());
}
