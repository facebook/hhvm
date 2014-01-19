<?php

$re = '{^(\s*\{\s*(?:"(?:\\\\["bfnrt/\\\\]|\\\\u[a-fA-F0-9]{4}|[^\0-\x09\x0a-\x1f\\\\"])*"\s*:\s*(?:[0-9.]+|null|true|false|"(?:\\\\["bfnrt/\\\\]|\\\\u[a-fA-F0-9]{4}|[^\0-\x09\x0a-\x1f\\\\"])*"|\[[^\]]*\]|\{(?:[^{}]*|\{(?:[^{}]*|\{(?:[^{}]*|\{(?:[^{}]*|\{[^{}]*\})*\})*\})*\})*\})\s*,\s*)*?)("require-dev"\s*:\s*(?:[0-9.]+|null|true|false|"(?:\\\\["bfnrt/\\\\]|\\\\u[a-fA-F0-9]{4}|[^\0-\x09\x0a-\x1f\\\\"])*"|\[[^\]]*\]|\{(?:[^{}]*|\{(?:[^{}]*|\{(?:[^{}]*|\{(?:[^{}]*|\{[^{}]*\})*\})*\})*\})*\}))(.*)}s';

$pass = '{
    "require": {
        "php": ">=5.3.3"
    },
    "require-dev": {
        "satooshi/php-coveralls": "dev-master"
    }
}';

$fail = '{
    "require": {
        "php": ">=5.3.3"
    }
}';

$count = preg_match($re, $pass, $match);
var_dump($count, preg_last_error() === PREG_BACKTRACK_LIMIT_ERROR);

$count = preg_match($re, $fail, $match);
var_dump($count, preg_last_error() === PREG_BACKTRACK_LIMIT_ERROR);

$count = preg_replace($re, '', $fail);
var_dump($count, preg_last_error() === PREG_BACKTRACK_LIMIT_ERROR);

$count = preg_replace($re, '', $pass);
var_dump($count, preg_last_error() === PREG_BACKTRACK_LIMIT_ERROR);

$count = preg_split($re, $fail);
var_dump($count, preg_last_error() === PREG_BACKTRACK_LIMIT_ERROR);

$count = preg_split($re, $pass);
var_dump($count, preg_last_error() === PREG_BACKTRACK_LIMIT_ERROR);
