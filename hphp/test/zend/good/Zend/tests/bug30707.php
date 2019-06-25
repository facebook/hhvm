<?hh
class C {
    function byePHP($plop) {
        echo "ok\n";
    }

    function plip() {
        try {
            $this->plap($this->plop());
        }    catch(Exception $e) {
        }
    }

    function plap($a) {
    }

    function plop() {
        throw new Exception;
    }
}
<<__EntryPoint>> function main(): void {
$x = new C;
$x->byePHP($x->plip());
}
