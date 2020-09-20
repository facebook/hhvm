<?hh <<__EntryPoint>>
function entrypoint_failedload(): void {
  simplexml_load_file('xhttp://example.com/');
}
