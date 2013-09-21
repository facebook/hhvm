<?php
$listenfd = socket_create(AF_INET6, SOCK_STREAM, SOL_TCP);
socket_bind($listenfd, "::1", 13579);
socket_listen($listenfd);
socket_set_nonblock($listenfd);
$connfd = @socket_accept($listenfd);
echo socket_last_error();