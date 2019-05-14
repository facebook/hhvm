<?php <<__EntryPoint>> function main() {
var_dump(
    strnatcmp('foo ', 'foo '),
    strnatcmp('foo', 'foo'),
    strnatcmp(' foo', ' foo')
);
}
