
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Create a function reference to an instance method on an object




``` Hack
namespace HH;

function inst_meth(
  $inst,
  string $meth_name,
);
```




The global function ` inst_meth($inst, 'meth_name') ` creates a reference
to an instance method on the specified object instance.




When using ` inst_meth ` all function calls will go to the single object
instance specified.  To call the same function on a collection
of objects of compatible types, use [` meth_caller `](/apis/Functions/HH/meth_caller/).




Hack provides a variety of methods that allow you to construct references to
methods for delegation.  The methods in this group are:




+ [` class_meth `](/apis/Functions/HH/class_meth/) for static methods on a class
+ [` fun `](/apis/Functions/HH/fun/) for global functions
+ [` inst_meth `](/apis/Functions/HH/inst_meth/) for instance methods on a single object
+ [` meth_caller `](/apis/Functions/HH/meth_caller/) for an instance method where the instance will be determined later
+ Or use anonymous code within a [lambda](</hack/functions/anonymous-functions>) expression.




# Example




```
<?hh
class C {
  public function isOdd(int $i): bool { return $i % 2 == 1; }
}

$C = new C();
$data = Vector { 1, 2, 3 };

// Each result returns Vector { 1, 3 }
var_dump($data->filter(inst_meth($C, 'isOdd')));
var_dump($data->filter($n ==> { return $C->isOdd($n); }));
```




## Parameters




* ` $inst ` The object whose method will be referenced.
* ` string $meth_name ` A constant string with the name of the instance method.




## Returns




- ` $func_ref ` - A fully typed function reference to the instance method.
<!-- HHAPIDOC -->
