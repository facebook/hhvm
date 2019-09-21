<?hh

<<__EntryPoint>>
function main_getimagesize() {
$image_data = "Qk06AAAAAAAAADYAAAAoAAAAAQAAAP////8BABgAAAA";
$image_data .= "AAAQAAADDDgAAww4AAAAAAAAAAAAA////AA==";
$info = null;
var_dump(getimagesizefromstring(base64_decode($image_data), inout $info));
}
