<?hh // in the default module

<<__EntryPoint>>
function main_basic_4() {
    foo(); // in module a (in the same deployment as the default module)
    bar(); // in module b (in a different deployment from the default module)
}
