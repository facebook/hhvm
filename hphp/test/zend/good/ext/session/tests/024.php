<?php
error_reporting(E_ALL);

class handler {
    public $data = 'baz|O:3:"foo":2:{s:3:"bar";s:2:"ok";s:3:"yes";i:1;}arr|a:1:{i:3;O:3:"foo":2:{s:3:"bar";s:2:"ok";s:3:"yes";i:1;}}';

    function open($save_path, $session_name)
    {
        print "OPEN: $session_name\n";
        return true;
    }
    function close()
    {
        return true;
    }
    function read($key)
    {
        print "READ: $key\n";
        return $GLOBALS["hnd"]->data;
    }

    function write($key, $val)
    {
        print "WRITE: $key, $val\n";
		$GLOBALS["hnd"]->data = $val;
        return true;
    }

    function destroy($key)
    {
        print "DESTROY: $key\n";
        return true;
    }

    function gc() { return true; }
}

$hnd = new handler;

class foo {
    public $bar = "ok";
    function method() { $this->yes++; }
}

session_set_save_handler(array($hnd, "open"), array($hnd, "close"), array($hnd, "read"), array($hnd, "write"), array($hnd, "destroy"), array($hnd, "gc"));

session_id("abtest");
session_start();

$baz = $_SESSION['baz'];
$arr = $_SESSION['arr'];
$baz->method();
$arr[3]->method();

var_dump($baz);
var_dump($arr);

session_write_close();

session_set_save_handler(array($hnd, "open"), array($hnd, "close"), array($hnd, "read"), array($hnd, "write"), array($hnd, "destroy"), array($hnd, "gc"));
session_start();

var_dump($baz);
var_dump($arr);

session_destroy();
?>
