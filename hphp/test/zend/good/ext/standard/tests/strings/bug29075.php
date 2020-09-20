<?hh <<__EntryPoint>> function main(): void {
var_dump(
    strnatcmp('foo ', 'foo '),
    strnatcmp('foo', 'foo'),
    strnatcmp(' foo', ' foo')
);
}
