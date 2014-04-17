<?php

$out = '';
foreach (range(0, 255) as $n)
{
    $url = 'http://example.org/' . chr($n);
    $out .= intval(false !== filter_var($url, FILTER_VALIDATE_URL));
}
echo wordwrap($out, 32, "\n", true), "\n";
