<?hh

function foo()
:mixed{
    print "foo";
}
<<__EntryPoint>> function main(): void {
register_shutdown_function(foo<>);

print "foo() will be called on shutdown...\n";
}
