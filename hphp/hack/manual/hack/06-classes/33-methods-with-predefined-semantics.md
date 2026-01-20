# Methods with Predefined Semantics

If a class contains a definition for a method having one of the following names, that method must have the prescribed visibility,
signature, and semantics:

Method Name     | Description
------------|-------------
[`__construct`](/hack/classes/constructors) | A constructor
[`__dispose`](#method-__dispose) | Performs object-cleanup
[`__disposeAsync`](#method-__disposeasync) | Performs object-cleanup
`__toString` | Returns a string representation of the instance on which it is called

## Method __construct

See [Constructors](/hack/classes/constructors).

## Method __dispose

This public instance method is required if the class implements the interface `IDisposable`; the method is intended to perform object
cleanup. The method's signature is, as follows:

```hack
class Example implements IDisposable {
  public function __dispose(): void {}
}
```

This method is called implicitly by the runtime when the instance goes out of scope, provided the attributes `<<__ReturnDisposable>>`
and `<<__AcceptDisposable>>` are *not* present.

See [object disposal](/hack/classes/object-disposal) for an example of its use and a discussion of these attributes.

## Method __disposeAsync

This public instance method is required if the class implements the interface `IAsyncDisposable`; the method is intended to perform
object cleanup. The method's signature is as follows:

```hack no-extract
public async function __disposeAsync(): Awaitable<void>;
```

This method is called implicitly by the runtime when the instance goes out of scope, provided the attributes `<<__ReturnDisposable>>`
and `<<__AcceptDisposable>>` are *not* present.

See [object disposal](/hack/classes/object-disposal) for a discussion of disposal and these attributes.
