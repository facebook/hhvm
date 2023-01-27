Hack arrays are value types. This makes your code easier to reason
about, faster (no work required on a fresh web request), and well
suited for caches.

If you really need mutation, Hack provides you with several options.

## Updating value types

Updating an element in a value type creates a new copy.

``` Hack
function update_value(vec<int> $items): vec<int> {
  // Both of these operations set $items a modified copy of the
  // original value.
  $items[0] = 42;
  $items[] = 100;
  return $items;
}

function demo(): void {
  $v = vec[1, 2];
  // $v is unaffected by this function call.
  $v2 = update_value($v);

  var_dump($v); // vec[1, 2]
  var_dump($v2); // vec[42, 2, 100]
}
```

Value types are shallow. A mutable type inside a value type is mutated
by reference.

``` Hack
class Person {
  public function __construct(public string $name) {}
}

function update_person(vec<Person> $items): void {
  $items[0]->name = "new";
}

function demo(): void {
  $v = vec[new Person("old")];

  update_person($v);

  var_dump($v); // vec[Person { name: "new" }]
}
```

## Using `inout` to simulate mutability

You can often use `inout` parameters instead of mutation. This copies
the modified parameters back to the locals of the caller.

``` Hack
function update_value(inout vec<int> $items): void {
  $items[0] = 42;
  $items[] = 100;
}

function demo(): void {
  $v = vec[1, 2];
  update_value(inout $v);

  var_dump($v); // vec[42, 2, 100]
}
```

## Using `Ref` for shallow mutability

The `Ref` class provides a single value that can be mutated.

```Hack no-extract
function update_value(Ref<vec<int>> $items): void {
  $inner = $items->get();
  $inner[0] = 42;
  $inner[] = 100;
  $items->set($inner);
}

function demo(): void {
  $v = new Ref(vec[1, 2]);
  update_value($v);

  var_dump($v->get()); // vec[42, 2, 100]
}

```

## Using Collections for mutability

All the Collection classes are mutable. We recommend using Hack arrays
wherever possible, but you can use `Vector`, `Set` or `Map` if you
really need to.

``` Hack
function update_value(Vector<int> $items): void {
  $items[0] = 42;
  $items[] = 100;
}

function demo(): void {
  $v = Vector {1, 2};
  update_value($v);

  var_dump($v); // Vector {42, 2, 100}
}
```
