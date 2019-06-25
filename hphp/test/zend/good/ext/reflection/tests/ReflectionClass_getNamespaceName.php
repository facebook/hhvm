<?hh
namespace A\B;
class Foo {
}
<<__EntryPoint>> function main(): void {
$function = new \ReflectionClass('stdClass');
\var_dump($function->inNamespace());
\var_dump($function->getName());
\var_dump($function->getNamespaceName());
\var_dump($function->getShortName());

$function = new \ReflectionClass('A\\B\\Foo');
\var_dump($function->inNamespace());
\var_dump($function->getName());
\var_dump($function->getNamespaceName());
\var_dump($function->getShortName());
}
