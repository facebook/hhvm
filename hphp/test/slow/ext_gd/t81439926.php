<?hh

<<__EntryPoint>>
function main(): void {
  $payload = "SUkqAAQAAAABgP////7//wCwAQKgQBYAAAAAAAAAAgIGkgIAAAAiAGgASgEAAP8A8okEAAAA8/Pz8/Pz80lJAQAAAAAAADACAP/Y/+wACOIIAQAiAP/sAAjiCAEAIgD/7AAICAEA/////wAAAAAI4ggADA==";
  exif_read_data("data://text/plain;base64,".$payload, "", false, true);
}
