<?php

function test($s1) { return $s1; }

function a() {
  $ret = fb_call_user_func_safe("TEst", "param");
  var_dump($ret);
}

function b() {
  $ret = fb_call_user_func_safe("NonTEst", "param");
  var_dump($ret);
}

function c() {
  $ret = fb_call_user_func_safe_return("TEst", "ok", "param");
  var_dump($ret);
}

function d() {
  $ret = fb_call_user_func_safe_return("NonTEst", "ok", "param");
  var_dump($ret);
}

function e() {
  $ret = fb_call_user_func_array_safe("TEst", "param");
  var_dump($ret);
}

function f() {
  $ret = fb_call_user_func_array_safe("NonTest", "param");
  var_dump($ret);
}

a();
b();
c();
d();
e();
f();

