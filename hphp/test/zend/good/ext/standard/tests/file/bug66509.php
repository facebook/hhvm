<?hh
<<__EntryPoint>> function main(): void {
$r = new \ReflectionFunction('copy');
 
foreach($r->getParameters() as $p) {
    var_dump($p->isOptional());	
}
}
