<?hh

class first {
    private function show() :mixed{
        echo "Call show()\n";
    }

    public function do_show() :mixed{
        $this->show();
    }
}

class second extends first {
}

class third extends second {
    private function show() :mixed{
        echo "Call show()\n";
    }
}

<<__EntryPoint>> function main(): void {
$t1 = new first();
$t1->do_show();

//$t2 = new second();
//$t2->do_show();

$t3 = new third();
$t3->do_show();

echo "Done\n";
}
