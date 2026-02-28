<?hh
<<__EntryPoint>> function main(): void {
// $ buck2 run //security/fbz/cmd/scepclient:scepclient -- --serial foo --serial bar --serial ASDF132 --cn "mslavin"  --challenge foo --upn "mslavin@meta.com" --csr_only
$csr_with_dns_names = <<<EOF
-----BEGIN CERTIFICATE REQUEST-----
MIIDIDCCAggCAQAwdjELMAkGA1UEBhMCVVMxEzARBgNVBAgTCkNhbGlmb3JuaWEx
EzARBgNVBAcTCk1lbmxvIFBhcmsxHDAaBgNVBAoTE01ldGEgUGxhdGZvcm1zIElu
Yy4xDTALBgNVBAsTBFRFU1QxEDAOBgNVBAMTB21zbGF2aW4wggEiMA0GCSqGSIb3
DQEBAQUAA4IBDwAwggEKAoIBAQDmZ+0x94OB0qnDCjEUVjsjwElQ7+wmMJMTIe9G
Gs4e7oTo2LddNQKIH9jUTkZXZmhK5lgGRqyagZjh0Kl+A8XCVG6Xu0lCsOubIoKs
i3iXykovEDAekNeIVc0VcWcE3JEqpTnFdatd59I/2Ikwr2tw8AEiS2x/XYq0e7Vz
CR6pYBGqdS3Cusch8PQ9K4sZ70Dr03dX8FLG/kmt1vIFsStUW0rP0BdW6vFaFyxx
7OIxrtzNNfmUfIx6AMSLatF/RQ9msffDvvHLbqne0LEnxxRhT88FSpGd2zvMUpEL
bhD4GfolCLD6VhaOPA8VaRN8abDSNx76vAd37PpzpN9eu8gNAgMBAAGgZTBPBgkq
hkiG9w0BCQ4xQjBAMD4GA1UdEQQ3MDWCA2Zvb4IDYmFyggdBU0RGMTMyoCAGCisG
AQQBgjcUAgOgEgwQbXNsYXZpbkBtZXRhLmNvbTASBgkqhkiG9w0BCQcxBRMDZm9v
MA0GCSqGSIb3DQEBCwUAA4IBAQCCvD1SE5NgIpBXS9HkaWzn0rJaF1dy/IKNeXch
8pNxgKvcqj4BtnjzOWPJxYC3g/H//VF+hkWkrsoO9WZqn0Ads4HeUSZUUhmFHYYi
U4ytI7WHN5o72DH3OEELwsR3UiirM5odU4H7d0jmPIa9SxRUsBc01bt5PdX3ia9F
dacL8wjAH2ZXGJ5zIMV4TXxV3M1w5hpa1zdceJKfSfC74Y/KO1hQ8wMS65NTpsDe
nkt89IwXeO8Rd5aK/HOPIX+IzQy+k0Px92CiAUp/KELtk3Unh2c0GDxMl26o/FNn
7/GpBbWxCH/fkFlmAJzQp9PRV8kAkselMp9LU3zvCceECSX7
-----END CERTIFICATE REQUEST-----
EOF;
// vec contains 3 DNS names
var_dump(openssl_csr_get_dns_names($csr_with_dns_names));
// $ buck2 run //security/fbz/cmd/scepclient:scepclient --  --cn "mslavin"  --challenge foo --upn "mslavin@meta.com" --csr_only
$csr_without_dns_names = <<<EOF
-----BEGIN CERTIFICATE REQUEST-----
MIIDDTCCAfUCAQAwdjELMAkGA1UEBhMCVVMxEzARBgNVBAgTCkNhbGlmb3JuaWEx
EzARBgNVBAcTCk1lbmxvIFBhcmsxHDAaBgNVBAoTE01ldGEgUGxhdGZvcm1zIElu
Yy4xDTALBgNVBAsTBFRFU1QxEDAOBgNVBAMTB21zbGF2aW4wggEiMA0GCSqGSIb3
DQEBAQUAA4IBDwAwggEKAoIBAQDaOjkmdJbTDZ8kKjEhheD0CildgEoXHRnkPR4g
H5kd+UGSopKTujPlQan2yguA7k4Sfdq4GywtCJSPZRwf6w+4UWH7UFUDGsxQ0iPE
4sOrx+IhVQO9qLJuJaqcNn77Fxp2VBrXOjYVz8u59MdHp3WuQpbLUZR0lAGoEiP9
F6QtCV6ztmJkGSIZI60MnbXS3maqBkCG3PzFEUN6Gn2xeIj1E6GVOaCKkTKiXPN8
Sm89ect0HpYjMan+mYUeriTZqVeDe4oXtYc8ekoefw/EiaCnLJA0xml7R179iFlF
ILkQUdbQmR59peu4J2r6KRiF6w1QUoF1wKmfD/pRy3/hOBHlAgMBAAGgUjA8Bgkq
hkiG9w0BCQ4xLzAtMCsGA1UdEQQkMCKgIAYKKwYBBAGCNxQCA6ASDBBtc2xhdmlu
QG1ldGEuY29tMBIGCSqGSIb3DQEJBzEFEwNmb28wDQYJKoZIhvcNAQELBQADggEB
ABkCuxc7PtNLwAdq75v7Pppmc/zbOj3AP/RSFokNqq3X0gvBiM+mmc7jjuTTIFnH
QQ0XPbO8vziW9CN9ksriJRhi5cHgQqOauEWBPgZoASWkSpDTtSl1wkDDkRZ8NEXC
RCgnd7kHFxrGZqmHwGxZG9vahkoKnbCAHQejkHBuf0tJziWzrqnb2RhlnN0J0nTa
/iJGVG95fmCEDh8QQWYOGNkh04WWZiLZV1Y7rU+fvu3qwJLSSVsTV+ekkcma1hT4
SIgtynnzh8U6TC063QNvSJWdYLnjsPFlnCZ5kKUbZ2cn2xdLy5tmNrh0j6ENnEwl
SCGR6wyTGCdbKQ9ZOD48qf8=
-----END CERTIFICATE REQUEST-----
EOF;
// vec contains no DNS names
var_dump(openssl_csr_get_dns_names($csr_without_dns_names));
try {
  var_dump(openssl_csr_get_dns_names("not a CSR"));
}
catch (OpenSSLException $e) {
  echo $e->getMessage()."\n";
}
}
