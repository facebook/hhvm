<?php

$LOG_ROOT = '/tmp/hhvm_server';
require __DIR__ . '/../../util/server_tests.inc';
require __DIR__ . '/../Adoy/Client.php';

$DOC_ROOT = dirname(__DIR__).'/server_root';

function runTest($testController, $customArgs = '') {
  global $DOC_ROOT;
  $pid = posix_getpid();
  try {
    $serverProc = $serverPort = $adminPort = null;
    $debugPort = false;
    $serverProc = startServer($serverPort, $adminPort, $debugPort,
                              dirname(__DIR__), $DOC_ROOT, $customArgs);
    $testController($serverPort);
    stopServer($adminPort, $serverProc);
  } catch (Exception $e) {
    error_log("Caught exception, test failed, pid=$pid, exn=".$e->getMessage());
    killChildren($pid);
    if ($serverProc) proc_close($serverProc);
    error_log('test failed');
  }
}

function request($host, $port, $path, $post = [], $headers = [], $extra_args = []) {
  global $test_run_id, $LOG_ROOT, $DOC_ROOT;

  try {
    $client = new \Adoy\FastCGI\Client($host, $port);
    $content = http_build_query($post);

    $args = array_merge(array(
      'GATEWAY_INTERFACE' => 'FastCGI/1.0',
      'REQUEST_METHOD' => empty($post) ? 'GET' : 'POST',
      'SERVER_SOFTWARE' => 'php/fcgiclient',
      'SERVER_PROTOCOL' => 'HTTP/1.1',
      'CONTENT_LENGTH' => strlen($content)
    ), $extra_args);
    foreach($headers as $k => $v) {
      $args["HTTP_" . strtoupper($k)] = $v;
    }
    $pos = strpos($path, '?');
    if ($pos !== false) {
      $args['QUERY_STRING'] = substr($path, $pos + 1);
      $path = substr($path, 0, $pos);
    }
    $doc_root = $DOC_ROOT;
    $pos = strpos($path, '//');
    if ($pos) {
      $doc_root .= '/' . substr($path, 0, $pos);
      $path = substr($path, $pos + 2);
    }
    $args['SCRIPT_FILENAME'] = $doc_root . '/' . $path;
    $args['SCRIPT_NAME'] = $path;
    $args['DOCUMENT_ROOT'] = $doc_root;

    @$result = $client->request($args, $content);

    $pos = strpos($result, "\r\n\r\n");
    if ($pos !== false) {
      $result = substr($result, $pos + 4);
    }
    return trim($result);
  } catch (Exception $e) {
    tlog("request exception: " . $e->getMessage());
    return "";
  }
}
