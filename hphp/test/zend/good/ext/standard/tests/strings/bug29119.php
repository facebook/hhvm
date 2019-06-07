<?hh <<__EntryPoint>> function main() {
var_dump(bin2hex(html_entity_decode('&ensp;&thinsp;&lsquo;&dagger;&prime;&frasl;&euro;', ENT_QUOTES, 'UTF-8')));
}
