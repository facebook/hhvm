<?hh

// Patched a self-signed certificate to include validity timestamps
// starting with nul byte
$cert  = "-----BEGIN CERTIFICATE-----\n";
$cert .= "MIIDiTCCAnGgAwIBAgIJALM0loMFrpTVMA0GCSqGSIb3DQEBBQUAMFsxCzAJBgNV\n";
$cert .= "BAYTAlVTMRUwEwYDVQQHDAxEZWZhdWx0IENpdHkxHDAaBgNVBAoME0RlZmF1bHQg\n";
$cert .= "Q29tcGFueSBMdGQxFzAVBgNVBAMMDkhIVk0gdGVzdCBjZXJ0MB4XDQCqqqqqqqqq\n";
$cert .= "qqqqqloXDQCqqqqqqqqqqqqqqlowWzELMAkGA1UEBhMCVVMxFTATBgNVBAcMDERl\n";
$cert .= "ZmF1bHQgQ2l0eTEcMBoGA1UECgwTRGVmYXVsdCBDb21wYW55IEx0ZDEXMBUGA1UE\n";
$cert .= "AwwOSEhWTSB0ZXN0IGNlcnQwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIB\n";
$cert .= "AQC8lY/Zf1d4JQ2FDd14a3UFCc8RfsuLHgVlIYwxoT+7rzulJ567OJrwbmKkTiTu\n";
$cert .= "qe177AjzaSpYX9CPR1PHcPTJfjE3mYeNu8Ei/aQTj1FkHj3s3kyZ1hvvVyAm1pWX\n";
$cert .= "as3v9KAEY8MBaGsZ3JFQndqEWDOl5VyUPDHFyficMJQqWicSMWalVs14OYCa7l8d\n";
$cert .= "6dLjJdg5MIBbPZySOAgGwmAZgiMzJd50doIiJtqxvnZk/N/ZeGKKYL9W+34Insv5\n";
$cert .= "d33h/iIhS7l0HVYDdyxQHonQVCNuu1CeXDL87VXMxJAQY5OFdIMvZ0NVNH+GXbOk\n";
$cert .= "evRSJqCrZ8mLA8lt7GIzRFixAgMBAAGjUDBOMB0GA1UdDgQWBBRIEoUUrpPgwi4r\n";
$cert .= "VMzXfIS8r0VRgDAfBgNVHSMEGDAWgBRIEoUUrpPgwi4rVMzXfIS8r0VRgDAMBgNV\n";
$cert .= "HRMEBTADAQH/MA0GCSqGSIb3DQEBBQUAA4IBAQBINpl5e+MinRrIgMqh+SxeD6PI\n";
$cert .= "cy/qOcSVxRVb1bM+V+m5xi3gtu0s+LieDjZRb1kanySwM6bJMcHEA+r9pg8ZOGkm\n";
$cert .= "AzvjtKe4OvFPv5L5J+3FtVZoKoGVXKHAKlt/A697ZLRfjXpi9eJ2UtoLTZVXPTSo\n";
$cert .= "jIuaGMoNJm6tzbbF3hCyjd1oPwPpOJgS5NF8j0TcTOcR8Ni0dSLgIHWhT5Wlh8+t\n";
$cert .= "5LU3pomjs+3yFeUyc7ZRg73Au/KjiKCsS32wsYHJ07OTkJz4xfixqpEYxZcO93Mb\n";
$cert .= "wxHscsNoSqih7zROMGEeLjLJSuvQZEA9r6ov1VyGClvkXjIrJArWfwO78QCK\n";
$cert .= "-----END CERTIFICATE-----\n";

$info = openssl_x509_parse($cert);
var_dump($info['validFrom_time_t']);
var_dump($info['validTo_time_t']);
var_dump($info['subject']['CN']);
