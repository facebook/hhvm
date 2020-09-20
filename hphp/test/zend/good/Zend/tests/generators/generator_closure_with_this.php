<?hh

class Test {
    public function getGenFactory() {
        return function() {
            yield $this;
        };
    }
}
<<__EntryPoint>> function main(): void {
$genFactory = (new Test)->getGenFactory();
$gen = $genFactory();
$gen->next();
var_dump($gen->current());
}
