<?php
class PHPUnit_Framework_MockObject_Stub_ReturnCallback {
    protected $callback;
    public function __construct($callback) {
        $this->callback = $callback;
    }
    public function invoke($invocation) {
        return call_user_func_array($this->callback, $invocation->parameters);
    }
}

class PHPUnit_Framework_MockObject_InvocationMocker {
    protected $matchers = [];
    public function addMatcher( $matcher) {
        $this->matchers[] = $matcher;
    }
    public function invoke( $invocation) {
        foreach ($this->matchers as $match) {
            $match->invoked($invocation);
        }
    }
}

class PHPUnit_Framework_MockObject_Matcher {
    public $stub = null;
    public function invoked($invocation) {
        return $this->stub->invoke($invocation);
    }
}

class MethodCallbackByReference {
    public function bar(&$a, &$b, $c) {
        Legacy::bar($a, $b, $c);
    }
    public function callback(&$a, &$b, $c) {
        $b = 1;
    }
}
class PHPUnit_Framework_MockObject_Invocation_Static {
    public $parameters;
    public function __construct(array $parameters) {
        $this->parameters = $parameters;
    }
}

class Mock_MethodCallbackByReference_7b180d26 extends MethodCallbackByReference {
    public $inv_mocker;
    public function bar(&$a, &$b, $c) {
        $arguments = array($a, $b, $c);
        $result = $this->inv_mocker->invoke(
            new PHPUnit_Framework_MockObject_Invocation_Static(
                $arguments
            )
        );
        return $result;
    }
}

set_error_handler(function() {
//    var_dump(func_get_args());
    DoesNotExists::$nope = true;
}, E_ALL | E_STRICT);

$foo = new Mock_MethodCallbackByReference_7b180d26();
$InvMocker = new PHPUnit_Framework_MockObject_InvocationMocker();
$foo->inv_mocker = $InvMocker;
$OuterMatcher = new PHPUnit_Framework_MockObject_Matcher();
$InvMocker->addMatcher($OuterMatcher);
$OuterMatcher->methodNameMatcher = null;
$OuterMatcher->stub = new PHPUnit_Framework_MockObject_Stub_ReturnCallback([$foo, 'callback']);
$a = $b = $c = 0;
$foo->bar($a, $b, $c);
