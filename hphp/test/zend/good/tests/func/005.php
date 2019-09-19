<?hh

function foo()
{
    print "foo";
}
<<__EntryPoint>> function main(): void {
register_shutdown_function(fun("foo"));

print "foo() will be called on shutdown...\n";
}
