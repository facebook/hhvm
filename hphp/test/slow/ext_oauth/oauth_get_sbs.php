<?hh
<<__EntryPoint>> function main(): void {
var_dump(oauth_get_sbs('GET', 'http://example.com/', dict['z' => 'y', 'a' => 'b']));
}
