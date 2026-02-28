<?hh
class C {
    function byePHP($plop) :mixed{
        echo "ok\n";
    }

    function plip() :mixed{
        try {
            $this->plap($this->plop());
        }    catch(Exception $e) {
        }
    }

    function plap($a) :mixed{
    }

    function plop() :mixed{
        throw new Exception;
    }
}
<<__EntryPoint>> function main(): void {
$x = new C;
$x->byePHP($x->plip());
}
