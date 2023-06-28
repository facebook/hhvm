<?hh
class testclass { function testfunc() :mixed{ } }
<<__EntryPoint>> function main(): void {
var_dump(method_exists('testclass','testfunc'));
var_dump(method_exists('testclass','nonfunc'));
}
