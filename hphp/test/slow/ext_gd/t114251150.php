<?hh

<<__EntryPoint>>
function main(): void {
  $s = "data://text/plain;base64,/9jhAGFFeGlmAABJSSoAIgAAAIYA/QAAAAACSSANRRCSAAAqYgQAO4i2ACphBAAliLb/IgAAAAQAAACth/vX/////////z0BAgYAAQAAAO/HxcoXAQj3AQAAABY1/5IiAAAAiA==";
  var_dump(exif_read_data($s, "", false, true));
}
