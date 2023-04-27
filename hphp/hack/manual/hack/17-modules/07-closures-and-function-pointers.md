Within a module, you can create a pointer to an internal function and pass it anywhere for use. Pointers to internal functions can only be created from within a module, but can be called and acccessed from anywhere.

```hack
//// newmodule.hack
new module foo {}

//// foo.hack
module foo;
internal function f() : void {
    echo "Internal f\n";
}
public function getF(): (function():void) {
    return f<>;
}
```

```hack no-extract
module bar;
public function test(): void {
    $f = getF();
    $f(); // prints "Internal f";
}
```

Similarly, a closure can access internal symbols from the module it's
defined in, but can be called from any location in code.

```hack
//// newmodule.hack
new module foo {}

//// foo.hack
module foo;
internal function f() : void {
    echo "Internal f\n";
}
public function getF(): (function():void) {
    return () ==> { f(); }; // ok
}
```

```hack no-extract
module bar;
public function test(): void {
    $f = getF();
    $f(); // prints "Internal f";
}
```

However, calling a function via string invocation must respect module boundaries. That is, if you try to call a function from outside of the current module using a string invocation, the runtime will throw an error. (The typechecker already bans this type of dynamic behavior).

```hack no-extract
module bar;
public function test(): void {
    $f = "f";
    $f(); // runtime error, f is internal to module foo
}
```
