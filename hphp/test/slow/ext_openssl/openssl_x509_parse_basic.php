<?hh

function normalize_certificate_data(dict<arraykey, mixed> $data): dict<arraykey, mixed> {
  unset($data['purposes'][10]);
  $authority_key = $data['extensions']['authorityKeyIdentifier'] ?? null;
  if ($authority_key is string) {
    $data['extensions']['authorityKeyIdentifier'] = rtrim($authority_key)."\n";
  }
  return $data;
}

<<__EntryPoint>> function main(): void {
$cert = "file://" . dirname(__FILE__) . "/cert.crt";

var_dump(normalize_certificate_data(openssl_x509_parse($cert)));
var_dump(normalize_certificate_data(openssl_x509_parse($cert, false)));
}
