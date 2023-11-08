/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <folly/io/async/EventBase.h>
#include <folly/io/async/SSLContext.h>
#include <folly/portability/GTest.h>
#include <glog/logging.h>
#include <wangle/acceptor/SSLContextSelectionMisc.h>
#include <wangle/ssl/SSLCacheOptions.h>
#include <wangle/ssl/SSLContextManager.h>
#include <wangle/ssl/ServerSSLContext.h>
#include <wangle/ssl/TLSTicketKeyManager.h>

#if defined(WANGLE_USE_FOLLY_TESTUTIL)
#include <folly/experimental/TestUtil.h>
#include <folly/io/async/test/TestSSLServer.h>

namespace {
std::string get_resource(const char* res) {
  return folly::test::find_resource(res).string();
}
} // namespace

using folly::test::kClientTestCert;
using folly::test::kClientTestChain;
using folly::test::kTestCert;
#else
namespace {
std::string get_resource(const char* res) {
  return res;
}
} // namespace

const char* kClientTestChain = "folly/io/async/test/certs/client_chain.pem";
const char* kClientTestCert = "folly/io/async/test/certs/client_cert.pem";
const char* kTestCert = "folly/io/async/test/certs/tests-cert.pem";
#endif

using std::shared_ptr;
using namespace folly;

namespace wangle {

// @lint-ignore-every PRIVATEKEY
static const std::string kTestCert1PEM = R"(
-----BEGIN CERTIFICATE-----
MIICFzCCAb6gAwIBAgIJAO6xBdXUFQqgMAkGByqGSM49BAEwaDELMAkGA1UEBhMC
VVMxFTATBgNVBAcMDERlZmF1bHQgQ2l0eTEcMBoGA1UECgwTRGVmYXVsdCBDb21w
YW55IEx0ZDERMA8GA1UECwwIdGVzdC5jb20xETAPBgNVBAMMCHRlc3QuY29tMCAX
DTE2MDMxNjE4MDg1M1oYDzQ3NTQwMjExMTgwODUzWjBoMQswCQYDVQQGEwJVUzEV
MBMGA1UEBwwMRGVmYXVsdCBDaXR5MRwwGgYDVQQKDBNEZWZhdWx0IENvbXBhbnkg
THRkMREwDwYDVQQLDAh0ZXN0LmNvbTERMA8GA1UEAwwIdGVzdC5jb20wWTATBgcq
hkjOPQIBBggqhkjOPQMBBwNCAARZ4vDgSPwytxU2HfQG/wxhsk0uHfr1eUmheqoC
yiQPB7aXZPbFs3JtvhzKc8DZ0rrZIQpkVLAGEIAa5UbuCy32o1AwTjAdBgNVHQ4E
FgQU05wwrHKWuyGM0qAIzeprza/FM9UwHwYDVR0jBBgwFoAU05wwrHKWuyGM0qAI
zeprza/FM9UwDAYDVR0TBAUwAwEB/zAJBgcqhkjOPQQBA0gAMEUCIBofo+kW0kxn
wzvNvopVKr/cFuDzwRKHdozoiZ492g6QAiEAo55BTcbSwBeszWR6Cr8gOCS4Oq7Z
Mt8v4GYjd1KT4fE=
-----END CERTIFICATE-----
)";

static const std::string kTestCert1Key = R"(
-----BEGIN EC PARAMETERS-----
BggqhkjOPQMBBw==
-----END EC PARAMETERS-----
-----BEGIN EC PRIVATE KEY-----
MHcCAQEEIKhuz+7RoCLvsXzcD1+Bq5ahrOViFJmgHiGR3w3OmXEroAoGCCqGSM49
AwEHoUQDQgAEWeLw4Ej8MrcVNh30Bv8MYbJNLh369XlJoXqqAsokDwe2l2T2xbNy
bb4cynPA2dK62SEKZFSwBhCAGuVG7gst9g==
-----END EC PRIVATE KEY-----
)";

static const std::string kTestCert2PEM = R"(
-----BEGIN CERTIFICATE-----
MIICHDCCAcOgAwIBAgIJAMXIoAvQSr5HMAoGCCqGSM49BAMCMGoxCzAJBgNVBAYT
AlVTMRUwEwYDVQQHDAxEZWZhdWx0IENpdHkxHDAaBgNVBAoME0RlZmF1bHQgQ29t
cGFueSBMdGQxEjAQBgNVBAsMCXRlc3QyLmNvbTESMBAGA1UEAwwJdGVzdDIuY29t
MCAXDTIwMDMxODIwNDI1NFoYDzMwMTkwNzIwMjA0MjU0WjBqMQswCQYDVQQGEwJV
UzEVMBMGA1UEBwwMRGVmYXVsdCBDaXR5MRwwGgYDVQQKDBNEZWZhdWx0IENvbXBh
bnkgTHRkMRIwEAYDVQQLDAl0ZXN0Mi5jb20xEjAQBgNVBAMMCXRlc3QyLmNvbTBZ
MBMGByqGSM49AgEGCCqGSM49AwEHA0IABLY1a1jMILAhlIvJS+G30h52LDnaeOvJ
SZf8SBV4kk0cx2/11wuA/Dw9auBOqadkhRI06cdT1SMfkxU+j0/Sh96jUDBOMB0G
A1UdDgQWBBRmOoWWWQR840qg207DzbHtUfmLZzAfBgNVHSMEGDAWgBRmOoWWWQR8
40qg207DzbHtUfmLZzAMBgNVHRMEBTADAQH/MAoGCCqGSM49BAMCA0cAMEQCIBYI
7R2QG2aBXqXi5YUkDYH140ZvWSVO72Ny8Vv0fHNUAiA8khaQGXyhSmg5XtdYf+95
FMG3ZdzUrVbeGa66iTqsKA==
-----END CERTIFICATE-----
)";

static const std::string kTestCert2Key = R"(
-----BEGIN PRIVATE KEY-----
MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgzgBUbZOZgJPOvfmZ
kfkqXA0kjCv+q9Mn4mSvnFZQ02ihRANCAAS2NWtYzCCwIZSLyUvht9Iediw52njr
yUmX/EgVeJJNHMdv9dcLgPw8PWrgTqmnZIUSNOnHU9UjH5MVPo9P0ofe
-----END PRIVATE KEY-----
)";

static const std::string kTestCert3PEM = R"(
-----BEGIN CERTIFICATE-----
MIICHTCCAcOgAwIBAgIJANhD01ZIjSaYMAoGCCqGSM49BAMCMGoxCzAJBgNVBAYT
AlVTMRUwEwYDVQQHDAxEZWZhdWx0IENpdHkxHDAaBgNVBAoME0RlZmF1bHQgQ29t
cGFueSBMdGQxEjAQBgNVBAsMCXRlc3QzLmNvbTESMBAGA1UEAwwJdGVzdDMuY29t
MCAXDTIwMDMxODIwNDM1M1oYDzMwMTkwNzIwMjA0MzUzWjBqMQswCQYDVQQGEwJV
UzEVMBMGA1UEBwwMRGVmYXVsdCBDaXR5MRwwGgYDVQQKDBNEZWZhdWx0IENvbXBh
bnkgTHRkMRIwEAYDVQQLDAl0ZXN0My5jb20xEjAQBgNVBAMMCXRlc3QzLmNvbTBZ
MBMGByqGSM49AgEGCCqGSM49AwEHA0IABPnM70rusTOR2a/6pp9ySifIak6E8OjG
OTInCWJinpcIL6/84dKkBbvnxoEnCac9D91Qn/DMS0SbFR+Ffy3eaJSjUDBOMB0G
A1UdDgQWBBSsgk2YknDXsMVAmPcNvmnsdQRe4DAfBgNVHSMEGDAWgBSsgk2YknDX
sMVAmPcNvmnsdQRe4DAMBgNVHRMEBTADAQH/MAoGCCqGSM49BAMCA0gAMEUCIHbT
lKFFkvhZk8ZA/R44o9uuUonJm5Gc4GrIU8FhprPyAiEA7X7y9w0wqBsRnqHY69/M
P1ay9D55cC8ZtIHW9Ioz4tU=
-----END CERTIFICATE-----
)";

static const std::string kTestCert3Key = R"(
-----BEGIN PRIVATE KEY-----
MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgVTwC3zm6JwlDVi/J
scDGImwGGxlgzHchexWJAsM/YNWhRANCAAT5zO9K7rEzkdmv+qafckonyGpOhPDo
xjkyJwliYp6XCC+v/OHSpAW758aBJwmnPQ/dUJ/wzEtEmxUfhX8t3miU
-----END PRIVATE KEY-----
)";

static const std::string kCertWithNoCNButWithSAN = R"(
-----BEGIN CERTIFICATE-----
MIIC4DCCAcigAwIBAgIJALTMdz3uPpCKMA0GCSqGSIb3DQEBCwUAMC8xLTArBgNV
BAoMJGludGVyb3AgcnVubmVyIENlcnRpZmljYXRlIEF1dGhvcml0eTAeFw0yMDA5
MjMxNjQyMzVaFw0yMTA5MjMxNjQyMzVaMBkxFzAVBgNVBAoMDmludGVyb3AgcnVu
bmVyMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA1mRHbCeMxpws+sHC
OFAALZpm0Lme6a2rmtAQDdNl+ZHj9jENaNpyqhtgmADQ4xe4eQ/3b+9Dzp5ZZttq
XQuJwSitVQkzk3C8fwEFxbU9lXmn2eU07EbNjc6yp+TNtfFiae5NaivuryeBGFAW
tZ7DfJW6u2nD4iGcCz8DGiWQ3A66yZr2F7vyww7L3IJgZJHLtNEJqyqNp/V6437y
e/av1MVptvyHF/x1G2C5sg7OmoL8Sh2eI9KSJ7cP8GK7AKUTGUNn86CUe1lshGof
sFxiOHUgOqGcWEC24G18rgt1vCKaxWDfTGO3pY+55bzFpnVdnBlK7kPG2+j+Bj8W
NwUEwQIDAQABoxUwEzARBgNVHREECjAIggZzZXJ2ZXIwDQYJKoZIhvcNAQELBQAD
ggEBAKawb0cI0Ieu19mk5LFkK74Hw7s4+58SucdC93zNBvitm1fnMDmCpYfFTsEV
LuuheCQGQSCwL5IMDXdhaHMefL1VTvkws/QOQWYcHB3M3SvC+nApkiSy7Z77+Q1i
oetv9zLnPX1l9wV2GozxygbLrFAICBNbfCBoES06ZrAVI5WKpRyH1iBN0O2oky8g
J6zWLo7/3QdkDXqxjPzAhe0QiLZj3gVbmb0qCDOQOb59WxfoXqH+0LYYWcz046p7
Qc0UmyO6jI4gPtR+E3C/U7qHpD0fkz2aVJp+vleOBtzinGfAhYPDgTCwQ55xGU72
MPZDaUDgJLPG+bgt0iK1AV7EBec=
-----END CERTIFICATE-----
)";

static const std::string kCertWithNoCNButWithSANKey = R"(
-----BEGIN PRIVATE KEY-----
MIIEwAIBADANBgkqhkiG9w0BAQEFAASCBKowggSmAgEAAoIBAQDWZEdsJ4zGnCz6
wcI4UAAtmmbQuZ7praua0BAN02X5keP2MQ1o2nKqG2CYANDjF7h5D/dv70POnllm
22pdC4nBKK1VCTOTcLx/AQXFtT2VeafZ5TTsRs2NzrKn5M218WJp7k1qK+6vJ4EY
UBa1nsN8lbq7acPiIZwLPwMaJZDcDrrJmvYXu/LDDsvcgmBkkcu00QmrKo2n9Xrj
fvJ79q/UxWm2/IcX/HUbYLmyDs6agvxKHZ4j0pIntw/wYrsApRMZQ2fzoJR7WWyE
ah+wXGI4dSA6oZxYQLbgbXyuC3W8IprFYN9MY7elj7nlvMWmdV2cGUruQ8bb6P4G
PxY3BQTBAgMBAAECggEBAJsCzGVVv0KG/zqbR6thpI9UeQxneY/pww7fawwkEjI9
mr6RvulWMNvviYq95EqeBwJ5WeWz8Kn+8hMdiC0YP5TKrXCzg3gSZifJ/HtzzMA7
wvIX+IjxtIPYtHISS+5GRmrjI1QlyaEZBg0nMxREY4G73NTO5xOkS2gSOlL4YGHK
bNdzQpLyZj6Goosr0g1J+l2/EVKO+/xBvZv6Fj6im+PmAimb+xe9iul7QDQHlYtI
T9J+2lvgzAof+uGYkMLwIO4R6xnjfd+Ey91rrQXgLfu1H63IT5qD0cOQ8sp+6XOW
slX6n1UxCjGYAJeSXqqu+5fX03XZt4h2zej2xtwLzeUCgYEA/RcnrLjzW2K1fnZq
WYuLcl9YwG4gUqqAzPsVOBPsWpnU1DMuC/L24u/CIlGLEEPRXgt8r6nGqfSg6Hs4
EOve8px3So5Nvt8EhugbANuQJlBPb3rkbV6cnCAbzpS6KYwCMNRZImO2K1bsab17
GBNagzKy5wqp6TFY0+CMySxytJ8CgYEA2Ns70YY3iYDFLLxrVpxHQG9bR4chATE2
tnePihfGgaW0oa744cMhJF/JIfpepB2JG/aOD6vmgtDO/z+vMqkO9Dvt/6wHa+S4
5AVkJnIkApNWXYdPM53szLx9fiuiQrQszsVpBSA20ytG9JjZ8/8i4T80KTgV0Il+
BrfzYYzjap8CgYEApCg8p59e2UtxBRGxcVs9m3WUj1vewz+sQ0goPzNM/ocAsJJx
r3ZsBE4W0UOqu2YBispQmW+5V8tAAwrJFtCmzx4FkeozKzZkLUynFytSmEdG/rvr
JbVURz/cSWXWSdRyJ1HUbqXWJs4+kWdBTCBheO+NcqZMBuDnCKaBKosV62kCgYEA
mvx9ES+Cy9Rrl6CGep0w012C+GkcbpS0zM5AmWqKpig/I/tAx2HFcxC+WHlvYI33
azPYrlymX8JK3uSuG1/2Xxnh7IQPvc33UoiD3nJfSDPzWt8U/QgWsPDaI+2dh1zs
VU+D0nUGmf/pM3F2/ErRq/iXGAqMlMFff//Cg2rFMnMCgYEA7De3L3aNpHFWhtUi
7c3DGtP3qTC6FAS9GJlA1TVsjgHt4nL3ZAKWOqXwVa1kXNDeYgN/Br39dfYW/KRE
Liz9kBryc02/2YSjg4YFDgrCqzeERPmwj8AKM3sPyL+nLzZOvJ3EbhyiQiOYGh+U
osLtL9tFEp8IrKHnoNVDuMPaZPM=
-----END PRIVATE KEY-----
)";

/**
 * CN = *.test4.com
 * SANS:
 *  - *.test4.com
 *  - test4.com
 *  - www.testtest4.com
 */
static const std::string kTestCert4PEM = R"(
-----BEGIN CERTIFICATE-----
MIIDsDCCApigAwIBAgIUYuo0xcXR8Y5C2+RqF2tStzkjmnYwDQYJKoZIhvcNAQEL
BQAwYzELMAkGA1UEBhMCVVMxCzAJBgNVBAgMAkNBMRMwEQYDVQQHDApNZW5sbyBQ
YXJrMQ0wCwYDVQQKDARNZXRhMQ0wCwYDVQQLDARUZXN0MRQwEgYDVQQDDAsqLnRl
c3Q0LmNvbTAgFw0yMzA1MzAyMTU3MTFaGA8yMDUwMTAxNTIxNTcxMVowYzELMAkG
A1UEBhMCVVMxCzAJBgNVBAgMAkNBMRMwEQYDVQQHDApNZW5sbyBQYXJrMQ0wCwYD
VQQKDARNZXRhMQ0wCwYDVQQLDARUZXN0MRQwEgYDVQQDDAsqLnRlc3Q0LmNvbTCC
ASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAM51A+Ay2e2bTzXcA6Umk9y/
0/87WMFG3YB0cFBdqN24HuxbV9PQ3+HA8QtUVgxnQToQIVvBodXMTOYe8KUuOl1X
8LVu6HkYwmyVt12loL4q9XTmYFII8mNIebgzFlMMf8ujmLpqpW62+1b7LxtsP0ES
rOXGJXHso1b5WAF41SCad3B3QUEIK/TiFNw9/LaLjBetZoWrE3WVJczzuSqZ3DJ3
8uh8dIa0xdCKAYgXkjhoPSapW9u5RzN6z58XAOjj0X8fXBKSF9bnP9C736H23YjP
7YaL1s/u3CEw6aX7GGg8v8zt6zdIgLRT8u8nQgFn8yAcnOzMLbJOiG2bUzya6FEC
AwEAAaNaMFgwCwYDVR0PBAQDAgQwMBMGA1UdJQQMMAoGCCsGAQUFBwMBMDQGA1Ud
EQQtMCuCCyoudGVzdDQuY29tggl0ZXN0NC5jb22CEXd3dy50ZXN0dGVzdDQuY29t
MA0GCSqGSIb3DQEBCwUAA4IBAQAPH/IQn2TXwQCd1JyhHDUdLNkEZlSdmirssus3
S0qH3U/BS2c7V+TuWFTh2HLXWDJFt/n7s3EEFBFOtJJtpu9o2fi2u3Qbx0sS8fde
JH3vq+qK7VpH4l4aSOW44xVWqGw2byhIL0pCw9W1KIlhTqCCm3SPD1NSF1vyS2e0
XMSc4vn6/2YwRwW7H6TjgeZQk7S3fK7mgg1tuzeA93P1I+GhwRXpiRnmUiOz2xWB
6FJGog2kEiakuw0LwR+61jSG4R3gCkkiHtRy+2OWZ0ugwERlNV8YzKHGBHJyDf7t
YSUTP0iLkFWK2J5ytvrjjpCxKiF83Qp2rUlaq+1GdBGrDnjC
-----END CERTIFICATE-----
)";

static const std::string kTestCert4Key = R"(
-----BEGIN PRIVATE KEY-----
MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQDOdQPgMtntm081
3AOlJpPcv9P/O1jBRt2AdHBQXajduB7sW1fT0N/hwPELVFYMZ0E6ECFbwaHVzEzm
HvClLjpdV/C1buh5GMJslbddpaC+KvV05mBSCPJjSHm4MxZTDH/Lo5i6aqVutvtW
+y8bbD9BEqzlxiVx7KNW+VgBeNUgmndwd0FBCCv04hTcPfy2i4wXrWaFqxN1lSXM
87kqmdwyd/LofHSGtMXQigGIF5I4aD0mqVvbuUczes+fFwDo49F/H1wSkhfW5z/Q
u9+h9t2Iz+2Gi9bP7twhMOml+xhoPL/M7es3SIC0U/LvJ0IBZ/MgHJzszC2yToht
m1M8muhRAgMBAAECggEBAMqXfOQmQj+tJb9eVJ1dC7+U5b0RSXjvxy/kEspp/ekE
YiPhRn/t+aOYJ3DMo1usfw8xAOr/SYV44wT124LbqB4sy2HeoXUjXLYc/ECC5Qd3
NEIwRth5OxE972NXjlKUc1srABX9zLSmDmE+Pu2T/UYnw03+cIQoh+gy6a8YGVvR
M7jZH3viSMImyeoWSCoaUPl212AywEdCY16WMJh+4Pg/3BO+P3b79eKAB9RunmED
Szc8l5czajFPKjo0BUKXNZQDE5qDws+picR6z5IN+YDLKbq+CTJPzaYsNkq2oxkI
fqnudhFZLuudbV6sXRvLHXVe9rcfgwKPrPJRpfBZLpkCgYEA9VuW6DSHsomR8LvD
QWsB09MwlQ47SIoJzdQGh4n7a6zCPBd6uD6e2F+RQLCPtu1emooxh6xhTPGWJLb8
LQUqdGAtgPJniIJrFqhRXKOMx8RakXPx+PdLcgKDuqqosS0nBH1REfNFT7Olx4oF
OdB9kOwWzDFIRTDECNPTH35KXP8CgYEA12l6q8ezNcDSGGuKACr9XQOwrwNl8070
xDhbaEOXc5dHBFq9A/gryUE2HFtWpMxtnzm5IBDza0h0vVcDu9PhAtNK5fBcP/kD
wRgHdI5oINa2G6IXSIk8JZqSq4rtNMfODlrjurfo+zZGfeQcZ3T+L8oaIfJeYKLG
+8JkFgX8qq8CgYA6L6xMCRkdOA7KHl0hyHS4fV8KGkPo4gONMnkR76EWWfP+ODs8
Mm2NNqzFwJl0cjp7P8abPEAe9TP8JQEM1CrLSnvAFryC3Rr0Vppk88xRG7m2wN5j
gpH7yajfvdUfn2ufXvC45w5K5nmsJokyusTsd3C4n/9ZmUUEKufokhSklQKBgDME
1nsNB1L54kjKX5r+k5eOIjCxW1ovHyXCO1QMfjhwYe+UFiR0iNpXyZsZvwG1MVl+
8Gu52A1W0d9uVoIMAsUKiji/nvm/3rXDHTDr8ZmlgOg2kdEqP8agP5DFjLztfc5E
lJ9Ko7Gr/zId7uRJ/1IOSfY0U0oMA5jpR483x8odAoGAV9DnSNb3snnEUCALDs9G
ezOhadBeQ+cLd2E0yXMKvib14hIOG6+mqVDRySz66AX5ekGBxSzFCKQ1pTMb2MjB
aK/EhwyLhJNHqVm2FtHxjmI4UNPSEOnRVtG6non09QgnTpNQucqOTxng3bPtpFOf
n3gKgnsMFz0yff/NT4tr8XE=
-----END PRIVATE KEY-----
)";

SSLContextManagerSettings& getSettings() {
  static SSLContextManagerSettings settings;
  return settings;
}

class SSLContextManagerForTest : public SSLContextManager {
 public:
  using SSLContextManager::insertSSLCtxByDomainName;
  using SSLContextManager::SSLContextManager;
};

TEST(SSLContextManagerTest, Test1) {
  SSLContextManagerForTest sslCtxMgr(
      "vip_ssl_context_manager_test_", getSettings(), nullptr);
  auto www_example_com_ctx = std::make_shared<SSLContext>();
  auto start_example_com_ctx = std::make_shared<SSLContext>();
  auto start_abc_example_com_ctx = std::make_shared<SSLContext>();

  sslCtxMgr.insertSSLCtxByDomainName("www.example.com", www_example_com_ctx);
  sslCtxMgr.insertSSLCtxByDomainName("www.example.com", www_example_com_ctx);
  sslCtxMgr.insertSSLCtxByDomainName("*.example.com", start_example_com_ctx);
  sslCtxMgr.insertSSLCtxByDomainName(
      "*.abc.example.com", start_abc_example_com_ctx);

  shared_ptr<SSLContext> retCtx;
  retCtx = sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("www.example.com"));
  EXPECT_EQ(retCtx, www_example_com_ctx);
  retCtx = sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("WWW.example.com"));
  EXPECT_EQ(retCtx, www_example_com_ctx);
  EXPECT_FALSE(
      sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("xyz.example.com")));

  retCtx = sslCtxMgr.getSSLCtxBySuffix(SSLContextKey("xyz.example.com"));
  EXPECT_EQ(retCtx, start_example_com_ctx);
  retCtx = sslCtxMgr.getSSLCtxBySuffix(SSLContextKey("XYZ.example.com"));
  EXPECT_EQ(retCtx, start_example_com_ctx);

  retCtx = sslCtxMgr.getSSLCtxBySuffix(SSLContextKey("www.abc.example.com"));
  EXPECT_EQ(retCtx, start_abc_example_com_ctx);

  // ensure "example.com" does not match "*.example.com"
  EXPECT_FALSE(sslCtxMgr.getSSLCtxBySuffix(SSLContextKey("example.com")));
  // ensure "Xexample.com" does not match "*.example.com"
  EXPECT_FALSE(sslCtxMgr.getSSLCtxBySuffix(SSLContextKey("Xexample.com")));
  // ensure wildcard name only matches one domain up
  EXPECT_FALSE(
      sslCtxMgr.getSSLCtxBySuffix(SSLContextKey("abc.xyz.example.com")));
}

// This test uses multiple contexts, which requires SNI support to work at all.
#if FOLLY_OPENSSL_HAS_SNI
TEST(SSLContextManagerTest, TestResetSSLContextConfigs) {
  SSLContextManagerForTest sslCtxMgr(
      "vip_ssl_context_manager_test_", getSettings(), nullptr);
  SSLCacheOptions cacheOptions;
  SocketAddress addr;

  TLSTicketKeySeeds seeds1{{"67"}, {"68"}, {"69"}};
  TLSTicketKeySeeds seeds2{{"68"}, {"69"}, {"70"}};
  TLSTicketKeySeeds seeds3{{"69"}, {"70"}, {"71"}};

  SSLContextConfig ctxConfig1;
  ctxConfig1.sessionContext = "ctx1";
  ctxConfig1.setCertificateBuf(kTestCert1PEM, kTestCert1Key);
  ctxConfig1.clientVerification =
      folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;
  SSLContextConfig ctxConfig1Default = ctxConfig1;
  ctxConfig1Default.isDefault = true;

  SSLContextConfig ctxConfig2;
  ctxConfig2.sessionContext = "ctx2";
  ctxConfig2.setCertificateBuf(kTestCert2PEM, kTestCert2Key);
  ctxConfig2.clientVerification =
      folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;
  SSLContextConfig ctxConfig2Default = ctxConfig2;
  ctxConfig2Default.isDefault = true;

  SSLContextConfig ctxConfig3;
  ctxConfig3.sessionContext = "ctx3";
  ctxConfig3.setCertificateBuf(kTestCert3PEM, kTestCert3Key);
  ctxConfig3.clientVerification =
      folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;
  SSLContextConfig ctxConfig3Default = ctxConfig3;
  ctxConfig3Default.isDefault = true;

  SSLContextConfig ctxConfig4;
  ctxConfig4.setCertificateBuf(kTestCert4PEM, kTestCert4Key);
  ctxConfig4.clientVerification =
      folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;

  SNIConfig sniConfig;
  SSLContextConfig ctxConfig4Override;
  ctxConfig4Override.domains = {"www.test4.com"};
  ctxConfig4Override.setCertificateBuf(kTestCert4PEM, kTestCert4Key);
  ctxConfig4Override.clientVerification =
      folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;
  sniConfig.snis = {"www.test4.com"};
  sniConfig.contextConfig = ctxConfig4Override;

  // Helper function that verifies seeds are what we expect.
  auto checkSeeds = [](std::shared_ptr<folly::SSLContext> ctx,
                       TLSTicketKeySeeds& seeds) {
    ASSERT_TRUE(ctx);
    auto ticketMgr =
        dynamic_cast<TLSTicketKeyManager*>(ctx->getTicketHandler());
    ASSERT_TRUE(ticketMgr);
    TLSTicketKeySeeds fetchedSeeds;
    ticketMgr->getTLSTicketKeySeeds(
        fetchedSeeds.oldSeeds,
        fetchedSeeds.currentSeeds,
        fetchedSeeds.newSeeds);
    EXPECT_EQ(fetchedSeeds, seeds);
  };

  // Reset with just one default
  sslCtxMgr.resetSSLContextConfigs(
      {ctxConfig1Default}, {}, cacheOptions, &seeds1, addr, nullptr);
  EXPECT_EQ(
      sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test.com")),
      sslCtxMgr.getDefaultSSLCtx());
  EXPECT_TRUE(sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test.com")));
  EXPECT_FALSE(sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test2.com")));
  EXPECT_FALSE(sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test3.com")));
  checkSeeds(
      sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test.com")), seeds1);

  // Reset with a different set of contexts, no new seeds.
  sslCtxMgr.resetSSLContextConfigs(
      {ctxConfig2Default, ctxConfig3},
      {sniConfig},
      cacheOptions,
      nullptr,
      addr,
      nullptr);
  EXPECT_FALSE(sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test.com")));
  EXPECT_TRUE(sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test2.com")));
  EXPECT_TRUE(sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test3.com")));
  EXPECT_TRUE(sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("www.test4.com")));
  checkSeeds(
      sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test2.com")), seeds1);
  checkSeeds(
      sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test3.com")), seeds1);
  checkSeeds(
      sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("www.test4.com")), seeds1);

  // New set of contexts, new seeds.
  sslCtxMgr.resetSSLContextConfigs(
      {ctxConfig1Default, ctxConfig3},
      {sniConfig},
      cacheOptions,
      &seeds2,
      addr,
      nullptr);
  EXPECT_TRUE(sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test.com")));
  EXPECT_FALSE(sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test2.com")));
  EXPECT_TRUE(sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test3.com")));
  EXPECT_TRUE(sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("www.test4.com")));
  checkSeeds(
      sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test.com")), seeds2);
  checkSeeds(
      sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test3.com")), seeds2);
  checkSeeds(
      sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("www.test4.com")), seeds2);

  // Back to one context, no new seeds.
  sslCtxMgr.resetSSLContextConfigs(
      {ctxConfig1Default}, {}, cacheOptions, nullptr, addr, nullptr);
  EXPECT_TRUE(sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test.com")));
  EXPECT_FALSE(sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test2.com")));
  EXPECT_FALSE(sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test3.com")));
  checkSeeds(
      sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test.com")), seeds2);

  // Finally, check that failure doesn't modify anything.
  // This will have new contexts + seeds, but two default contexts set. This
  // should error.
  EXPECT_THROW(
      sslCtxMgr.resetSSLContextConfigs(
          {ctxConfig1Default, ctxConfig2Default, ctxConfig3},
          {},
          cacheOptions,
          &seeds3,
          addr,
          nullptr),
      std::runtime_error);
  // These should return the same as the previous successful result
  EXPECT_TRUE(sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test.com")));
  EXPECT_FALSE(sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test2.com")));
  EXPECT_FALSE(sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test3.com")));
  checkSeeds(
      sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test.com")), seeds2);
}
#endif

#if !(FOLLY_OPENSSL_IS_110) && !defined(OPENSSL_IS_BORINGSSL)
TEST(SSLContextManagerTest, TestSessionContextIfSupplied) {
  SSLContextManagerForTest sslCtxMgr(
      "vip_ssl_context_manager_test_", getSettings(), nullptr);
  SSLContextConfig ctxConfig;
  ctxConfig.sessionContext = "test";
  ctxConfig.addCertificateBuf(kTestCert1PEM, kTestCert1Key);

  SSLCacheOptions cacheOptions;
  SocketAddress addr;

  sslCtxMgr.addSSLContextConfig(
      ctxConfig, cacheOptions, nullptr, addr, nullptr);

  using namespace std::string_literals;
  auto ctx = sslCtxMgr.getSSLCtx("test.com"s);
  ASSERT_NE(ctx, nullptr);
  auto sessCtxFromCtx = std::string(
      reinterpret_cast<char*>(ctx->getSSLCtx()->sid_ctx),
      ctx->getSSLCtx()->sid_ctx_length);
  EXPECT_EQ(*ctxConfig.sessionContext, sessCtxFromCtx);
}

TEST(SSLContextManagerTest, TestSessionContextIfSessionCacheAbsent) {
  SSLContextManagerForTest sslCtxMgr(
      "vip_ssl_context_manager_test_", getSettings(), nullptr);
  SSLContextConfig ctxConfig;
  ctxConfig.sessionContext = "test";
  ctxConfig.sessionCacheEnabled = false;
  ctxConfig.addCertificateBuf(kTestCert1PEM, kTestCert1Key);

  SSLCacheOptions cacheOptions;
  SocketAddress addr;

  sslCtxMgr.addSSLContextConfig(
      ctxConfig, cacheOptions, nullptr, addr, nullptr);

  using namespace std::string_literals;
  auto ctx = sslCtxMgr.getSSLCtx("test.com"s);
  ASSERT_NE(ctx, nullptr);
  auto sessCtxFromCtx = std::string(
      reinterpret_cast<char*>(ctx->getSSLCtx()->sid_ctx),
      ctx->getSSLCtx()->sid_ctx_length);
  EXPECT_EQ(*ctxConfig.sessionContext, sessCtxFromCtx);
}
#endif

TEST(SSLContextManagerTest, TestSessionContextCertRemoval) {
  SSLContextManagerForTest sslCtxMgr(
      "vip_ssl_context_manager_test_", getSettings(), nullptr);
  auto www_example_com_ctx = std::make_shared<ServerSSLContext>();
  auto start_example_com_ctx = std::make_shared<ServerSSLContext>();
  auto start_abc_example_com_ctx = std::make_shared<ServerSSLContext>();
  auto www_abc_example_com_ctx = std::make_shared<ServerSSLContext>();

  sslCtxMgr.insertSSLCtxByDomainName("www.example.com", www_example_com_ctx);
  sslCtxMgr.insertSSLCtxByDomainName("*.example.com", start_example_com_ctx);
  sslCtxMgr.insertSSLCtxByDomainName(
      "*.abc.example.com", start_abc_example_com_ctx);

  shared_ptr<SSLContext> retCtx;
  retCtx = sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("www.example.com"));
  EXPECT_EQ(retCtx, www_example_com_ctx);
  retCtx = sslCtxMgr.getSSLCtxBySuffix(SSLContextKey("www.abc.example.com"));
  EXPECT_EQ(retCtx, start_abc_example_com_ctx);
  retCtx = sslCtxMgr.getSSLCtxBySuffix(SSLContextKey("xyz.example.com"));
  EXPECT_EQ(retCtx, start_example_com_ctx);

  // Removing one of the contexts
  sslCtxMgr.removeSSLContextConfig(SSLContextKey("www.example.com"));
  retCtx = sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("www.example.com"));
  EXPECT_FALSE(retCtx);

  // If the wildcard context is successfully removed, there should be no context
  // for a random domain that is of the form *.example.com.
  sslCtxMgr.removeSSLContextConfig(SSLContextKey(".example.com"));
  using namespace std::string_literals;
  retCtx = sslCtxMgr.getSSLCtx("foo.example.com"s);
  EXPECT_FALSE(retCtx);

  // Add it back and delete again but with the other API.
  sslCtxMgr.insertSSLCtxByDomainName("*.example.com", start_example_com_ctx);
  retCtx = sslCtxMgr.getSSLCtx("foo.example.com"s);
  EXPECT_TRUE(retCtx);
  sslCtxMgr.removeSSLContextConfigByDomainName("*.example.com");
  retCtx = sslCtxMgr.getSSLCtx("foo.example.com"s);
  EXPECT_FALSE(retCtx);

  // Try to remove the context which does not exist - must be NOOP
  sslCtxMgr.removeSSLContextConfig(SSLContextKey("xyz.example.com"));

  // Setting a default context
  sslCtxMgr.insertSSLCtxByDomainName(
      "www.abc.example.com", www_abc_example_com_ctx, true);

  // Context Manager must throw on attempt to remove the default context
  EXPECT_THROW(
      sslCtxMgr.removeSSLContextConfig(SSLContextKey("www.abc.example.com")),
      std::invalid_argument);
}

TEST(SSLContextManagerTest, TestCertificateWithNoCN) {
  SSLContextManagerForTest sslCtxMgr(
      "vip_ssl_context_manager_test_", getSettings(), nullptr);
  SSLContextConfig ctxConfig;
  ctxConfig.sessionContext = "ctx";
  ctxConfig.setCertificateBuf(
      kCertWithNoCNButWithSAN, kCertWithNoCNButWithSANKey);
  ctxConfig.isDefault = true;
  ctxConfig.clientVerification =
      folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;
  SSLCacheOptions cacheOptions;
  SocketAddress addr;
  sslCtxMgr.addSSLContextConfig(
      ctxConfig, cacheOptions, nullptr, addr, nullptr);
  using namespace std::string_literals;
  auto ctx = sslCtxMgr.getSSLCtx("O = interop runner"s);
  ASSERT_NE(ctx, nullptr);
}

TEST(SSLContextManagerTest, TestAlpnAllowMismatch) {
  SSLContextManagerForTest sslCtxMgr(
      "vip_ssl_context_manager_test_", getSettings(), nullptr);
  SSLContextConfig ctxConfig;
  ctxConfig.alpnAllowMismatch = true;
  ctxConfig.sessionContext = "ctx";
  ctxConfig.setCertificateBuf(
      kCertWithNoCNButWithSAN, kCertWithNoCNButWithSANKey);
  ctxConfig.isDefault = true;
  ctxConfig.clientVerification =
      folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;
  SSLCacheOptions cacheOptions;
  SocketAddress addr;
  sslCtxMgr.addSSLContextConfig(
      ctxConfig, cacheOptions, nullptr, addr, nullptr);
  using namespace std::string_literals;
  auto ctx = sslCtxMgr.getSSLCtx("O = interop runner"s);
  ASSERT_NE(ctx, nullptr);
  ASSERT_TRUE(ctx->getAlpnAllowMismatch());
}

TEST(SSLContextManagerTest, TestAlpnNotAllowMismatch) {
  SSLContextManagerForTest sslCtxMgr(
      "vip_ssl_context_manager_test_", getSettings(), nullptr);
  SSLContextConfig ctxConfig;
  ctxConfig.alpnAllowMismatch = false;
  ctxConfig.sessionContext = "ctx";
  ctxConfig.setCertificateBuf(
      kCertWithNoCNButWithSAN, kCertWithNoCNButWithSANKey);
  ctxConfig.isDefault = true;
  ctxConfig.clientVerification =
      folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;
  SSLCacheOptions cacheOptions;
  SocketAddress addr;
  sslCtxMgr.addSSLContextConfig(
      ctxConfig, cacheOptions, nullptr, addr, nullptr);
  using namespace std::string_literals;
  auto ctx = sslCtxMgr.getSSLCtx("O = interop runner"s);
  ASSERT_NE(ctx, nullptr);
  ASSERT_FALSE(ctx->getAlpnAllowMismatch());
}

TEST(SSLContextManagerTest, TestSingleClientCAFileSet) {
  SSLContextManagerForTest sslCtxMgr(
      "vip_ssl_context_manager_test_", getSettings(), nullptr);
  const std::string clientCAFile = get_resource(kClientTestChain);

  SSLContextConfig ctxConfig;
  ctxConfig.clientCAFile = clientCAFile;
  ctxConfig.clientVerification =
      folly::SSLContext::VerifyClientCertificate::ALWAYS;
  ctxConfig.sessionContext = "test";
  ctxConfig.isDefault = true;
  ctxConfig.setCertificateBuf(kTestCert1PEM, kTestCert1Key);

  SSLCacheOptions cacheOptions;
  SocketAddress addr;
  sslCtxMgr.addSSLContextConfig(
      ctxConfig, cacheOptions, nullptr, addr, nullptr);
  auto ctx = sslCtxMgr.getDefaultSSLCtx();
  ASSERT_NE(ctx, nullptr);

  STACK_OF(X509_NAME)* names = SSL_CTX_get_client_CA_list(ctx->getSSLCtx());
  EXPECT_EQ(2, sk_X509_NAME_num(names));

  static const char* kExpectedCNs[] = {"Leaf Certificate", "Intermediate CA"};
  for (int i = 0; i < sk_X509_NAME_num(names); i++) {
    auto name = sk_X509_NAME_value(names, i);
    int indexCN = X509_NAME_get_index_by_NID(name, NID_commonName, -1);
    EXPECT_NE(indexCN, -1);

    auto entry = X509_NAME_get_entry(name, indexCN);
    ASSERT_NE(entry, nullptr);
    auto asnStringCN = X509_NAME_ENTRY_get_data(entry);
    std::string commonName(
        reinterpret_cast<const char*>(ASN1_STRING_get0_data(asnStringCN)),
        ASN1_STRING_length(asnStringCN));
    EXPECT_EQ(commonName, std::string(kExpectedCNs[i]));
  }
}

TEST(SSLContextManagerTest, TestMultipleClientCAsSet) {
  SSLContextManagerForTest sslCtxMgr(
      "vip_ssl_context_manager_test_", getSettings(), nullptr);
  const std::vector<std::string> clientCAFiles{
      get_resource(kClientTestCert), get_resource(kTestCert)};

  SSLContextConfig ctxConfig;
  ctxConfig.clientCAFiles = clientCAFiles;
  ctxConfig.clientVerification =
      folly::SSLContext::VerifyClientCertificate::ALWAYS;
  ctxConfig.sessionContext = "test";
  ctxConfig.isDefault = true;
  ctxConfig.setCertificateBuf(kTestCert1PEM, kTestCert1Key);

  SSLCacheOptions cacheOptions;
  SocketAddress addr;
  sslCtxMgr.addSSLContextConfig(
      ctxConfig, cacheOptions, nullptr, addr, nullptr);
  auto ctx = sslCtxMgr.getDefaultSSLCtx();
  ASSERT_NE(ctx, nullptr);

  STACK_OF(X509_NAME)* names = SSL_CTX_get_client_CA_list(ctx->getSSLCtx());
  EXPECT_EQ(2, sk_X509_NAME_num(names));

  static const char* kExpectedCNs[] = {"testuser1", "Asox Company"};
  for (int i = 0; i < sk_X509_NAME_num(names); i++) {
    auto name = sk_X509_NAME_value(names, i);
    int indexCN = X509_NAME_get_index_by_NID(name, NID_commonName, -1);
    EXPECT_NE(indexCN, -1);

    auto entry = X509_NAME_get_entry(name, indexCN);
    ASSERT_NE(entry, nullptr);
    auto asnStringCN = X509_NAME_ENTRY_get_data(entry);
    std::string commonName(
        reinterpret_cast<const char*>(ASN1_STRING_get0_data(asnStringCN)),
        ASN1_STRING_length(asnStringCN));
    EXPECT_EQ(commonName, std::string(kExpectedCNs[i]));
  }
}

TEST(SSLContextManagerTest, TestDomainAssociatedSSLContextConfigs) {
  using namespace std::string_literals;
  SSLContextManagerForTest sslCtxMgr(
      "vip_ssl_context_manager_test_",
      SSLContextManagerSettings().setEnableSNICallback(false),
      nullptr);
  SSLCacheOptions cacheOptions;
  SocketAddress addr;
  TLSTicketKeySeeds seeds{{"67"}, {"68"}, {"69"}};

  std::vector<std::string> domains1 = {"*.test4.com", "test4.com"};
  SSLContextConfig ctxConfig1;
  ctxConfig1.domains = {"*.test4.com"};
  ctxConfig1.setCertificateBuf(kTestCert4PEM, kTestCert4Key);
  ctxConfig1.clientVerification =
      folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;
  ctxConfig1.isDefault = true;

  std::vector<std::string> domains2 = {"www.testtest4.com"};
  SSLContextConfig ctxConfig2;
  ctxConfig2.domains = domains2;
  ctxConfig2.setCertificateBuf(kTestCert4PEM, kTestCert4Key);
  ctxConfig2.clientVerification =
      folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;

  auto configs = {
      std::pair{domains1, ctxConfig1}, std::pair{domains2, ctxConfig2}};
  for (const auto& config : configs) {
    sslCtxMgr.addSSLContextConfig(
        config.first, config.second, cacheOptions, &seeds, addr, nullptr);
  }

  auto ctx1 = sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey(".test4.com"));
  auto ctx2 =
      sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("www.testtest4.com"));
  EXPECT_EQ(sslCtxMgr.getDefaultSSLCtx(), ctx1);
  EXPECT_NE(ctx1, ctx2);
  EXPECT_EQ(ctx1, sslCtxMgr.getSSLCtx("foo.test4.com"s));
  EXPECT_EQ(nullptr, sslCtxMgr.getSSLCtx("foo.unknown.com"s));
}

TEST(
    SSLContextManagerTest,
    TestDomainsAssociatedSSLContextConfigsCompatibility) {
  using namespace std::string_literals;
  SSLContextManagerForTest sslCtxMgr(
      "vip_ssl_context_manager_test_",
      SSLContextManagerSettings().setEnableSNICallback(false),
      nullptr);
  SSLCacheOptions cacheOptions;
  SocketAddress addr;
  TLSTicketKeySeeds seeds{{"67"}, {"68"}, {"69"}};

  SSLContextConfig ctxConfig1;
  ctxConfig1.setCertificateBuf(kTestCert1PEM, kTestCert1Key);
  ctxConfig1.clientVerification =
      folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;
  ctxConfig1.isDefault = true;

  SSLContextConfig ctxConfig2;
  ctxConfig2.setCertificateBuf(kTestCert2PEM, kTestCert2Key);
  ctxConfig2.clientVerification =
      folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;

  SSLContextConfig ctxConfig3;
  ctxConfig3.setCertificateBuf(kTestCert3PEM, kTestCert3Key);
  ctxConfig3.clientVerification =
      folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;

  SSLContextConfig ctxConfig4;
  ctxConfig4.setCertificateBuf(kTestCert4PEM, kTestCert4Key);
  ctxConfig4.clientVerification =
      folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;

  SNIConfig sniConfig;
  SSLContextConfig ctxConfig4Override;
  ctxConfig4Override.domains = {"www.test4.com"};
  ctxConfig4Override.setCertificateBuf(kTestCert4PEM, kTestCert4Key);
  ctxConfig4Override.clientVerification =
      folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;
  sniConfig.snis = {"www.test4.com"};
  sniConfig.contextConfig = ctxConfig4Override;

  sslCtxMgr.resetSSLContextConfigs(
      {ctxConfig1, ctxConfig2, ctxConfig3, ctxConfig4},
      {sniConfig},
      cacheOptions,
      &seeds,
      addr,
      nullptr);

  auto ctx = sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("www.test4.com"));
  EXPECT_NE(ctx, sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey(".test4.com")));
  EXPECT_EQ(
      sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey(".test4.com")),
      sslCtxMgr.getSSLCtx("foo.test4.com"s));
}

TEST(SSLContextManagerTest, TestNoSNIContextConfiguration) {
  using namespace std::string_literals;
  SSLContextManagerForTest sslCtxMgr(
      "vip_ssl_context_manager_test_",
      SSLContextManagerSettings().setEnableSNICallback(false),
      nullptr);
  SSLCacheOptions cacheOptions;
  SocketAddress addr;
  TLSTicketKeySeeds seeds{{"67"}, {"68"}, {"69"}};

  SSLContextConfig ctxConfig;
  ctxConfig.domains = {"www.test4.com"};
  ctxConfig.setCertificateBuf(kTestCert4PEM, kTestCert4Key);
  ctxConfig.clientVerification =
      folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;
  ctxConfig.isDefault = true;

  SNIConfig sniConfig;
  SSLContextConfig noSNIContextConfig = ctxConfig;
  noSNIContextConfig.isDefault = false;
  sniConfig.contextConfig = noSNIContextConfig;
  ;

  sslCtxMgr.resetSSLContextConfigs(
      {ctxConfig}, {sniConfig}, cacheOptions, &seeds, addr, nullptr);

  EXPECT_EQ(
      sslCtxMgr.getSSLCtx("www.test4.com"s), sslCtxMgr.getDefaultSSLCtx());
  EXPECT_NE(nullptr, sslCtxMgr.getNoSNICtx());
  EXPECT_NE(sslCtxMgr.getNoSNICtx(), sslCtxMgr.getDefaultSSLCtx());

  sslCtxMgr.resetSSLContextConfigs(
      {ctxConfig}, {}, cacheOptions, &seeds, addr, nullptr);

  EXPECT_EQ(nullptr, sslCtxMgr.getNoSNICtx());
}
} // namespace wangle
