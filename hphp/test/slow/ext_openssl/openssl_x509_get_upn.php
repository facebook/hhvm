<?hh
<<__EntryPoint>> function main(): void {
/*
How to generate these test certificates from a CSR containing the desired extensions/values
$ cat >openssl.cnf <<EOF
[ ca ]
default_ca              = CA_default
[ CA_default ]
copy_extensions         = copyall
default_md              = sha256
default_days            = 1
new_certs_dir           = .
database                = ca.idx
serial                  = ca.srl
policy                  = POLICY_any
[ POLICY_any ]
countryName             = optional
stateOrProvinceName     = optional
organizationName        = optional
organizationalUnitName  = optional
commonName              = supplied
emailAddress            = optional
EOF
$ touch ca.idx
$ echo 01 > ca.srl
$ openssl genrsa -out hphp_test_ca_key.pem 4096
$ openssl req -x509 -sha256 -new -nodes -key hphp_test_ca_key.pem -days 3650 -out hphp_test_ca_cert.pem
$ openssl ca -config openssl.cnf -cert hphp_test_ca_cert.pem -keyfile hphp_test_ca_key.pem -out hphp_test_cert.pem -in csr.pem
*/

$cert_with_dns_names_and_upn = <<<EOF
-----BEGIN CERTIFICATE-----
MIIEsTCCApmgAwIBAgIBATANBgkqhkiG9w0BAQsFADBRMQswCQYDVQQGEwJYWDEV
MBMGA1UEBwwMRGVmYXVsdCBDaXR5MRwwGgYDVQQKDBNEZWZhdWx0IENvbXBhbnkg
THRkMQ0wCwYDVQQDDARURVNUMB4XDTI1MDkyNzAyMzAxMloXDTI1MDkyODAyMzAx
MlowYTELMAkGA1UEBhMCVVMxEzARBgNVBAgTCkNhbGlmb3JuaWExHDAaBgNVBAoT
E01ldGEgUGxhdGZvcm1zIEluYy4xDTALBgNVBAsTBFRFU1QxEDAOBgNVBAMTB21z
bGF2aW4wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDmZ+0x94OB0qnD
CjEUVjsjwElQ7+wmMJMTIe9GGs4e7oTo2LddNQKIH9jUTkZXZmhK5lgGRqyagZjh
0Kl+A8XCVG6Xu0lCsOubIoKsi3iXykovEDAekNeIVc0VcWcE3JEqpTnFdatd59I/
2Ikwr2tw8AEiS2x/XYq0e7VzCR6pYBGqdS3Cusch8PQ9K4sZ70Dr03dX8FLG/kmt
1vIFsStUW0rP0BdW6vFaFyxx7OIxrtzNNfmUfIx6AMSLatF/RQ9msffDvvHLbqne
0LEnxxRhT88FSpGd2zvMUpELbhD4GfolCLD6VhaOPA8VaRN8abDSNx76vAd37Ppz
pN9eu8gNAgMBAAGjgYMwgYAwPgYDVR0RBDcwNYIDZm9vggNiYXKCB0FTREYxMzKg
IAYKKwYBBAGCNxQCA6ASDBBtc2xhdmluQG1ldGEuY29tMB0GA1UdDgQWBBRHmn77
9xr9pxtPxIAP45qn1slmfTAfBgNVHSMEGDAWgBTwdI1xiPvAcpmCmLiXrkfQUGzd
PDANBgkqhkiG9w0BAQsFAAOCAgEAAn30tLxFMPwAy0i1ogAu1DoS2vo3LKgjUAjZ
VXKbEPpsGHH5RG7h822ATdPoYwdECvpK7qBbUMYX/z1736cJTrateP/h1byeGpOz
HrSn3j7dURWbEz3XD+ZDQDjBOTsHAaKueg3FHfHe1q9WY7PmJt35Sofrsz8GdcKb
A/NhS9tCkmyWFsAYch5byiXxHlSfj20r/v+HEFbPEZ9UPwy2DWNF92Z2DXW98Fd4
FW6DQCYK3jQ8Y9k+gWxtTBND4uRMJ3DY9tE/f066FVrFGs166NBgrlE1kE2w226s
dNsdw4JxabMi1nP3M6TKiDtIKm06eDNG51EHN9GvJNpmfKSp1AquV9byN6J2V0iT
Nbh4jw4WU5GX+QHoj3TCSB2HZ6V8G9tQfeHvbsnmDNuWhY+I8Kwiw9r/vUZPgh3A
goKbp0sPD3M1Ye3twa566BOiLhI3WfY3XuWQNN63wljxlthMGkMYapbJ/df1buJP
L/fXx02uUoFzG8G8ZS7mC41Wu+TaoPaGyhTAWKmJ2iAefzzdr+3KSm5X8KnKNIlf
yXAbSBpanqkSdUv+GQOsAzL15Qt1vUEaQAY9Ldyg7E1dTyP6kf/5cQ/NswnEl2fF
6moLdovHI4vpW/cPl0kJv7v3llEiid7e/6fbxT+Zg6dG+u34dbb08GAQ8rZauu5E
lKDaMTI=
-----END CERTIFICATE-----
EOF;
// UPN printed
var_dump(openssl_x509_get_upn($cert_with_dns_names_and_upn));
$cert_with_upn_without_dns_names = <<<EOF
-----BEGIN CERTIFICATE-----
MIIEnDCCAoSgAwIBAgIBATANBgkqhkiG9w0BAQsFADBRMQswCQYDVQQGEwJYWDEV
MBMGA1UEBwwMRGVmYXVsdCBDaXR5MRwwGgYDVQQKDBNEZWZhdWx0IENvbXBhbnkg
THRkMQ0wCwYDVQQDDARURVNUMB4XDTI1MDkyNzAyMzgxMVoXDTI1MDkyODAyMzgx
MVowYTELMAkGA1UEBhMCVVMxEzARBgNVBAgTCkNhbGlmb3JuaWExHDAaBgNVBAoT
E01ldGEgUGxhdGZvcm1zIEluYy4xDTALBgNVBAsTBFRFU1QxEDAOBgNVBAMTB21z
bGF2aW4wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaOjkmdJbTDZ8k
KjEhheD0CildgEoXHRnkPR4gH5kd+UGSopKTujPlQan2yguA7k4Sfdq4GywtCJSP
ZRwf6w+4UWH7UFUDGsxQ0iPE4sOrx+IhVQO9qLJuJaqcNn77Fxp2VBrXOjYVz8u5
9MdHp3WuQpbLUZR0lAGoEiP9F6QtCV6ztmJkGSIZI60MnbXS3maqBkCG3PzFEUN6
Gn2xeIj1E6GVOaCKkTKiXPN8Sm89ect0HpYjMan+mYUeriTZqVeDe4oXtYc8ekoe
fw/EiaCnLJA0xml7R179iFlFILkQUdbQmR59peu4J2r6KRiF6w1QUoF1wKmfD/pR
y3/hOBHlAgMBAAGjbzBtMCsGA1UdEQQkMCKgIAYKKwYBBAGCNxQCA6ASDBBtc2xh
dmluQG1ldGEuY29tMB0GA1UdDgQWBBQKQLxHIkSDG7UHMgSZNZEBopuQgjAfBgNV
HSMEGDAWgBTwdI1xiPvAcpmCmLiXrkfQUGzdPDANBgkqhkiG9w0BAQsFAAOCAgEA
Kf+SDBLMUKs+VI1Uy3bJdgDOwz1v1P3ASNoLhpW1S413bkXFSmve7+wzEQR9wtxX
lNdnrmZHFlhaM2g97BTPPA/agW4u3oQ90kePz1uFCq/+KJqne6MbnJ0bkAlwUKsM
wSrZplqqKUnFX2SWRVaQgjXm7LxEKAl0XdAyTXNqen1Lhyb7OeBudtcwQ6DiLR9H
j8aHNI/HNKQanvySYMIwFEu7qIWFC14igBziOGQAurX9gUsJvPuyMMq9BvQXNS9Q
3gwrTZPTlwXAqvrhm0YfmViIJXrOqo8kqM3xC3aL2K4v+1rL3stgsMQwtU0H3Hgw
haBHPzufLsrmznH7hXJSNEl9ELVu9pqaNfWeEqfcRYFNieoVaAtkUCev+6kB+jOf
0l3UCwJkYZkuK6JrCLACxnce8G250we2GprcH7bPNOVhXZlRidQd1X+mudA67CSd
ir7Ca/FJony7/b4/DTFUwLYhCwGZgezJblcVdUG/T7AEBCFIWpfxqmkcWNsYAUts
cP8f3TyGhS1sZvIXt017gKdY6nqk7YoIQsYKQi7kZYJlCpIcYJj9NvT/gm5KDABQ
qeHuCAOmkNeOGbvF9je2wT5ST0Q2R8zZuGljuIFQ1g4yAWES589YrhIe+tM9+q4R
LXG5OhPW804mq+e+3si5GZ9DyMMQN83AiYpF5tXOzwU=
-----END CERTIFICATE-----
EOF;
// UPN printed
var_dump(openssl_x509_get_upn($cert_with_upn_without_dns_names));
// get one of these
$cert_with_dns_names_no_upn = <<<EOF
-----BEGIN CERTIFICATE-----
MIIEgTCCAmmgAwIBAgIBATANBgkqhkiG9w0BAQsFADBRMQswCQYDVQQGEwJYWDEV
MBMGA1UEBwwMRGVmYXVsdCBDaXR5MRwwGgYDVQQKDBNEZWZhdWx0IENvbXBhbnkg
THRkMQ0wCwYDVQQDDARURVNUMB4XDTI1MDkyNzAyNDEyM1oXDTI1MDkyODAyNDEy
M1owYTELMAkGA1UEBhMCVVMxEzARBgNVBAgTCkNhbGlmb3JuaWExHDAaBgNVBAoT
E01ldGEgUGxhdGZvcm1zIEluYy4xDTALBgNVBAsTBFRFU1QxEDAOBgNVBAMTB21z
bGF2aW4wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC+3VQXmJtjSzvt
QnAaykWEeT0F0LZc9Z5tBAZ+3JXZU6YUKcdqhfWjXqzCOguQ2cAssk1SVX/LUwKQ
YzLn5vU+aB7FIAEm39hKqKl4s7JeLy4uzYo1Od44GHMGqn8Xsna+dctOu67CbSci
t0XQ3kt7Ec/RLcu87qSNRMgGxSVB3vQ9ewTjSoCMS5sLqFf7XEQZApyMj82gMYz7
ddgVZzW4Tna+B8B4MVpXq5VzJAuYNPr07CFV3GvIFkmdWftYXiXqmSa+YfKfdSYw
+3EOwj8XAacHx0X2GMATkk7g5EBVoqCA3aqKLE0w5uxThDMz+JSVScHV9HCPE3Gr
GqGekOuJAgMBAAGjVDBSMBAGA1UdEQQJMAeCBTEyMzQ1MB0GA1UdDgQWBBSco1+q
Yojkw0Nrviw3NcoLOmu9nDAfBgNVHSMEGDAWgBTwdI1xiPvAcpmCmLiXrkfQUGzd
PDANBgkqhkiG9w0BAQsFAAOCAgEAhiQXdVwcy6kztR1nNTMoIeEoZXWTAWRZzZZd
cNoSmIvLIc9pxq+FMGIKJ5IAtlupg9lW3iXvD5FXw7TVedvCXq2SHIkj7viTzoYZ
HAEsSG8n5N2wsdjjagtlubzDovzLFkfRaEuxCV4XgaLX49gfQfTI0feCEgPqWsRC
u4BM1lyuTYEAh7txzXAtMIGDFKERr3Bq8BNbHBhIWashopx67lkMbXuSXOQ7kjBr
CLiLdW6L6dQTLhwFFB/ncsunLIYLQvLgXwmp4HXnVevGWVgRlq2NDka/Lj9SjgGW
aEd1km6c4/dFt05C/YqX4V4lgILw9LScVLQKqo0lNN7rOjcn3+c0R+rAus+wonXY
1wG54VqXkzplg9wPzwG7YvSapWr33c1lkisv7s5ErT9nVevrtsDD2BgTlgO2EqV1
ZnWToXgGhU9wUVfNPXP/h3oCK/2kIUFr4pSAGrXklm+vvWUFk2JrrQ5qRiaBGxQd
dALGjdKKUaSWDmfLiQx2fQklzUMRSdHg5q00GCpxwL84nVJFlIQ0YcThmf98cBEU
d6YDw3VRI3ZI3fVddmNzgpsYTY+JdMW5InHDiwembfW3JJbowkbXwhpllqeg9Ve8
OovcG5JlLMro2K6yPkz5EQwsVkYLx3PLFERjfdqDSThTUuWZ/43GuXFfdDTCJXtR
6lSsekU=
-----END CERTIFICATE-----
EOF;
// no UPN in this certificate
try {
  var_dump(openssl_x509_get_upn($cert_with_dns_names_no_upn));
}
catch (OpenSSLException $e) {
  echo $e->getMessage()."\n";
}
try {
  var_dump(openssl_x509_get_upn("not a certificate"));
}
catch (OpenSSLException $e) {
  echo $e->getMessage()."\n";
}
}
