<?php

class pear_error {
    public function pear_error($message = 'unknown error', $code = null,
                               $mode = null, $options = null,
                               $userinfo = null) {}

    public function addUserInfo($info) {}
    public function getCallback() {}
    public function getCode() {}
    public function getMessage() {}
    public function getMode() {}
    public function getDebugInfo() {}
    public function getType() {}
    public function getUserInfo() {}
    public function getBacktrace($frame = null) {}

    public function toString() {}
}
