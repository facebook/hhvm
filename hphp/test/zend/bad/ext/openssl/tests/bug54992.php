<?php
$context = stream_context_create();

stream_context_set_option($context, 'ssl', 'local_cert', __DIR__ . "/bug54992.pem");
stream_context_set_option($context, 'ssl', 'allow_self_signed', true);
$server = stream_socket_server('ssl://127.0.0.1:64321', $errno, $errstr,
	STREAM_SERVER_BIND|STREAM_SERVER_LISTEN, $context);


$pid = pcntl_fork();
if ($pid == -1) {
	die('could not fork');
} else if ($pid) {
	$contextC = stream_context_create(
		array(
			'ssl' => array(
				'verify_peer'		=> true,
				'cafile'		=> __DIR__ . '/bug54992-ca.pem',
				'CN_match'		=> 'buga_buga',
			)
		)
	);
	var_dump(stream_socket_client("ssl://127.0.0.1:64321", $errno, $errstr, 1,
		STREAM_CLIENT_CONNECT, $contextC));
} else {	
	@pcntl_wait($status);
	@stream_socket_accept($server, 1);
}