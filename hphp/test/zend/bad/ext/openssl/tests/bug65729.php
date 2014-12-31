<?php
$serverCode = <<<'CODE'
    $serverUri = "ssl://127.0.0.1:64321";
    $serverFlags = STREAM_SERVER_BIND | STREAM_SERVER_LISTEN;
    $serverCtx = stream_context_create(['ssl' => [
        'local_cert' => __DIR__ . '/bug65729.pem'
    ]]);

    $server = stream_socket_server($serverUri, $errno, $errstr, $serverFlags, $serverCtx);
    phpt_notify();

    $expected_names = ['foo.test.com.sg', 'foo.test.com', 'FOO.TEST.COM', 'foo.bar.test.com'];
    foreach ($expected_names as $name) {
        @stream_socket_accept($server, 1);
    }
CODE;

$clientCode = <<<'CODE'
    $serverUri = "ssl://127.0.0.1:64321";
    $clientFlags = STREAM_CLIENT_CONNECT;

    phpt_wait();

    $expected_names = ['foo.test.com.sg', 'foo.test.com', 'FOO.TEST.COM', 'foo.bar.test.com'];
    foreach ($expected_names as $expected_name) {
        $clientCtx = stream_context_create(['ssl' => [
            'verify_peer'       => true,
            'allow_self_signed'    => true,
            'peer_name'          => $expected_name,
        ]]);

        var_dump(stream_socket_client($serverUri, $errno, $errstr, 2, $clientFlags, $clientCtx));
    }
CODE;

include 'ServerClientTestCase.inc';
ServerClientTestCase::getInstance()->run($clientCode, $serverCode);
