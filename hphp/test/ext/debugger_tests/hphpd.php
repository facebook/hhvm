<?php

define('FAIL', 'fail');
define('PASS', 'pass');

class TestFailure extends Exception {
  private $got, $exp;
  public function __construct($got, $exp) {
    $this->got = $got;
    $this->exp = $exp;
  }
  public function __toString() {
    return "Expect: '".var_export($this->exp, true).
      "'\nGot: '".var_export($this->got, true)."'\n".
           $this->getTraceAsString();
  }
}

function VS($got, $exp) {
  if ($got !== $exp) {
    throw new TestFailure($got, $exp);
  }
}

function get_client($name, $user) {
  if (!function_exists("hphpd_get_client")) {
    error_log("hphpd_get_client undefined");
    return false;
  }
  $client = hphpd_get_client($name);
  if (!$client) {
    error_log("client $name is grabbed by another request");
    return false;
  }
  if ($client->getState() == DebuggerClient::STATE_UNINIT) {
    $options = array("user" => $user);
    $client->init($options);
  }
  if ($client->getState() != DebuggerClient::STATE_READY_FOR_COMMAND) {
    error_log("client not ready, likely failed to initialize");
    return false;
  }
  return $client;
}

// Wait for the debugger client identified by $name to enter the busy state.
// This ensures the client has continued the process and is ready to receive
// interrupts.
function waitForClientToGetBusy($name) {
  if (!function_exists("hphpd_client_ctrl")) {
    error_log("hphpd_client_ctrl undefined");
    return false;
  }
  while (($result = hphpd_client_ctrl($name, "getstate")) !=
         DebuggerClient::STATE_BUSY) {
    error_log("Waiting for client $name to get busy. Current state $result");
    sleep(1);
  }
  return true;
}

// Interrupt the debugger client identified by $name.
function interrupt($name) {
  // Wait for the client to enter the busy state before sending the interrupt,
  // otherwise the interrupt would be wasted.
  if (waitForClientToGetBusy($name)) {
    error_log("interrupting client $name in state $result");
    hphpd_client_ctrl($name, "interrupt");
  }
}

function show($x) {
  $str = var_export($x, true);
  error_log($str);
}

function check($name) {
  error_log(hphpd_client_ctrl($name, "getstate"));
}

