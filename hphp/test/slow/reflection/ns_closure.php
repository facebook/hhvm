<?hh

namespace foo;


<<__EntryPoint>>
function main_ns_closure() :mixed{
$a = function () {};
$r = new \ReflectionFunction($a);
\var_dump($r->inNamespace());
\var_dump($r->getNamespaceName());
\var_dump($r->getName());
}
