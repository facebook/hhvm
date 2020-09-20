<?hh

class Meta {
}

class CParent extends Meta {
    public $publicVar;
    protected $protectedVar;
}

class Child extends CParent {
}
<<__EntryPoint>> function main(): void {
$r = new ReflectionClass('Child');

var_dump($r->getProperty('publicVar')->getDeclaringClass()->getName());
var_dump($r->getProperty('protectedVar')->getDeclaringClass()->getName());

echo "Done\n";
}
