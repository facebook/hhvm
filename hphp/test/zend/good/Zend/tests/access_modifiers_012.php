<?php
class C {
	protected function prot() { }
	private function priv() { }
	public function __call($name, $args)    {
        echo "In __call() for method $name()\n";
    }
}

$c = new C;
call_user_func(array($c, 'none'));
call_user_func(array($c, 'prot'));
call_user_func(array($c, 'priv'));
?>