<?php
$strings = array(
    'SW1wbGVtZW50YXRpb25zIE1VU1QgcmVqZWN0IHRoZSBlbmNvZGluZyBpZiBpdCBjb250YWlucyBjaGFyYWN0ZXJzIG91dHNpZGUgdGhlIGJhc2UgYWxwaGFiZXQu',
    'SW1wbGVtZW$0YXRpb25zIE1VU1QgcmVqZWN0IHRoZSBlbmNvZGluZyBpZiBpdCBjb250YWlucyBjaGFyYWN0ZXJzIG91dHNpZGUgdGhlIGJhc2UgYWxwaGFiZXQu',
    'SW1wbGVtZW0YXRpb25zIE1VU1QgcmVqZWN0IHRoZSBlbmNvZGluZyBpZiBpdCBjb250YWlucyBjaGFyYWN0ZXJzIG91dHNpZGUgdGhlIGJhc2UgYWxwaGFiZXQu'
);
foreach($strings as $string) {
    var_dump(base64_decode($string, true));
}
?>