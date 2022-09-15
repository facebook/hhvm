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

using std::shared_ptr;
using namespace folly;

// @lint-ignore-every PRIVATEKEY

namespace wangle {

static const std::string kTestCert1PEM{
    "-----BEGIN CERTIFICATE-----\n"
    "MIICFzCCAb6gAwIBAgIJAO6xBdXUFQqgMAkGByqGSM49BAEwaDELMAkGA1UEBhMC\n"
    "VVMxFTATBgNVBAcMDERlZmF1bHQgQ2l0eTEcMBoGA1UECgwTRGVmYXVsdCBDb21w\n"
    "YW55IEx0ZDERMA8GA1UECwwIdGVzdC5jb20xETAPBgNVBAMMCHRlc3QuY29tMCAX\n"
    "DTE2MDMxNjE4MDg1M1oYDzQ3NTQwMjExMTgwODUzWjBoMQswCQYDVQQGEwJVUzEV\n"
    "MBMGA1UEBwwMRGVmYXVsdCBDaXR5MRwwGgYDVQQKDBNEZWZhdWx0IENvbXBhbnkg\n"
    "THRkMREwDwYDVQQLDAh0ZXN0LmNvbTERMA8GA1UEAwwIdGVzdC5jb20wWTATBgcq\n"
    "hkjOPQIBBggqhkjOPQMBBwNCAARZ4vDgSPwytxU2HfQG/wxhsk0uHfr1eUmheqoC\n"
    "yiQPB7aXZPbFs3JtvhzKc8DZ0rrZIQpkVLAGEIAa5UbuCy32o1AwTjAdBgNVHQ4E\n"
    "FgQU05wwrHKWuyGM0qAIzeprza/FM9UwHwYDVR0jBBgwFoAU05wwrHKWuyGM0qAI\n"
    "zeprza/FM9UwDAYDVR0TBAUwAwEB/zAJBgcqhkjOPQQBA0gAMEUCIBofo+kW0kxn\n"
    "wzvNvopVKr/cFuDzwRKHdozoiZ492g6QAiEAo55BTcbSwBeszWR6Cr8gOCS4Oq7Z\n"
    "Mt8v4GYjd1KT4fE=\n"
    "-----END CERTIFICATE-----\n"};

static const std::string kTestCert1Key{
    "-----BEGIN EC PARAMETERS-----\n"
    "BggqhkjOPQMBBw==\n"
    "-----END EC PARAMETERS-----\n"
    "-----BEGIN EC PRIVATE KEY-----\n"
    "MHcCAQEEIKhuz+7RoCLvsXzcD1+Bq5ahrOViFJmgHiGR3w3OmXEroAoGCCqGSM49\n"
    "AwEHoUQDQgAEWeLw4Ej8MrcVNh30Bv8MYbJNLh369XlJoXqqAsokDwe2l2T2xbNy\n"
    "bb4cynPA2dK62SEKZFSwBhCAGuVG7gst9g==\n"
    "-----END EC PRIVATE KEY-----\n"};

static const std::string kTestCert2PEM{
    "-----BEGIN CERTIFICATE-----\n"
    "MIICHDCCAcOgAwIBAgIJAMXIoAvQSr5HMAoGCCqGSM49BAMCMGoxCzAJBgNVBAYT\n"
    "AlVTMRUwEwYDVQQHDAxEZWZhdWx0IENpdHkxHDAaBgNVBAoME0RlZmF1bHQgQ29t\n"
    "cGFueSBMdGQxEjAQBgNVBAsMCXRlc3QyLmNvbTESMBAGA1UEAwwJdGVzdDIuY29t\n"
    "MCAXDTIwMDMxODIwNDI1NFoYDzMwMTkwNzIwMjA0MjU0WjBqMQswCQYDVQQGEwJV\n"
    "UzEVMBMGA1UEBwwMRGVmYXVsdCBDaXR5MRwwGgYDVQQKDBNEZWZhdWx0IENvbXBh\n"
    "bnkgTHRkMRIwEAYDVQQLDAl0ZXN0Mi5jb20xEjAQBgNVBAMMCXRlc3QyLmNvbTBZ\n"
    "MBMGByqGSM49AgEGCCqGSM49AwEHA0IABLY1a1jMILAhlIvJS+G30h52LDnaeOvJ\n"
    "SZf8SBV4kk0cx2/11wuA/Dw9auBOqadkhRI06cdT1SMfkxU+j0/Sh96jUDBOMB0G\n"
    "A1UdDgQWBBRmOoWWWQR840qg207DzbHtUfmLZzAfBgNVHSMEGDAWgBRmOoWWWQR8\n"
    "40qg207DzbHtUfmLZzAMBgNVHRMEBTADAQH/MAoGCCqGSM49BAMCA0cAMEQCIBYI\n"
    "7R2QG2aBXqXi5YUkDYH140ZvWSVO72Ny8Vv0fHNUAiA8khaQGXyhSmg5XtdYf+95\n"
    "FMG3ZdzUrVbeGa66iTqsKA==\n"
    "-----END CERTIFICATE-----\n"};

static const std::string kTestCert2Key{
    "-----BEGIN PRIVATE KEY-----\n"
    "MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgzgBUbZOZgJPOvfmZ\n"
    "kfkqXA0kjCv+q9Mn4mSvnFZQ02ihRANCAAS2NWtYzCCwIZSLyUvht9Iediw52njr\n"
    "yUmX/EgVeJJNHMdv9dcLgPw8PWrgTqmnZIUSNOnHU9UjH5MVPo9P0ofe\n"
    "-----END PRIVATE KEY-----\n"};

static const std::string kTestCert3PEM{
    "-----BEGIN CERTIFICATE-----\n"
    "MIICHTCCAcOgAwIBAgIJANhD01ZIjSaYMAoGCCqGSM49BAMCMGoxCzAJBgNVBAYT\n"
    "AlVTMRUwEwYDVQQHDAxEZWZhdWx0IENpdHkxHDAaBgNVBAoME0RlZmF1bHQgQ29t\n"
    "cGFueSBMdGQxEjAQBgNVBAsMCXRlc3QzLmNvbTESMBAGA1UEAwwJdGVzdDMuY29t\n"
    "MCAXDTIwMDMxODIwNDM1M1oYDzMwMTkwNzIwMjA0MzUzWjBqMQswCQYDVQQGEwJV\n"
    "UzEVMBMGA1UEBwwMRGVmYXVsdCBDaXR5MRwwGgYDVQQKDBNEZWZhdWx0IENvbXBh\n"
    "bnkgTHRkMRIwEAYDVQQLDAl0ZXN0My5jb20xEjAQBgNVBAMMCXRlc3QzLmNvbTBZ\n"
    "MBMGByqGSM49AgEGCCqGSM49AwEHA0IABPnM70rusTOR2a/6pp9ySifIak6E8OjG\n"
    "OTInCWJinpcIL6/84dKkBbvnxoEnCac9D91Qn/DMS0SbFR+Ffy3eaJSjUDBOMB0G\n"
    "A1UdDgQWBBSsgk2YknDXsMVAmPcNvmnsdQRe4DAfBgNVHSMEGDAWgBSsgk2YknDX\n"
    "sMVAmPcNvmnsdQRe4DAMBgNVHRMEBTADAQH/MAoGCCqGSM49BAMCA0gAMEUCIHbT\n"
    "lKFFkvhZk8ZA/R44o9uuUonJm5Gc4GrIU8FhprPyAiEA7X7y9w0wqBsRnqHY69/M\n"
    "P1ay9D55cC8ZtIHW9Ioz4tU=\n"
    "-----END CERTIFICATE-----\n"};

static const std::string kTestCert3Key{
    "-----BEGIN PRIVATE KEY-----\n"
    "MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgVTwC3zm6JwlDVi/J\n"
    "scDGImwGGxlgzHchexWJAsM/YNWhRANCAAT5zO9K7rEzkdmv+qafckonyGpOhPDo\n"
    "xjkyJwliYp6XCC+v/OHSpAW758aBJwmnPQ/dUJ/wzEtEmxUfhX8t3miU\n"
    "-----END PRIVATE KEY-----\n"};

static const std::string kCertWithNoCNButWithSAN{
    "-----BEGIN CERTIFICATE-----\n"
    "MIIC4DCCAcigAwIBAgIJALTMdz3uPpCKMA0GCSqGSIb3DQEBCwUAMC8xLTArBgNV\n"
    "BAoMJGludGVyb3AgcnVubmVyIENlcnRpZmljYXRlIEF1dGhvcml0eTAeFw0yMDA5\n"
    "MjMxNjQyMzVaFw0yMTA5MjMxNjQyMzVaMBkxFzAVBgNVBAoMDmludGVyb3AgcnVu\n"
    "bmVyMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA1mRHbCeMxpws+sHC\n"
    "OFAALZpm0Lme6a2rmtAQDdNl+ZHj9jENaNpyqhtgmADQ4xe4eQ/3b+9Dzp5ZZttq\n"
    "XQuJwSitVQkzk3C8fwEFxbU9lXmn2eU07EbNjc6yp+TNtfFiae5NaivuryeBGFAW\n"
    "tZ7DfJW6u2nD4iGcCz8DGiWQ3A66yZr2F7vyww7L3IJgZJHLtNEJqyqNp/V6437y\n"
    "e/av1MVptvyHF/x1G2C5sg7OmoL8Sh2eI9KSJ7cP8GK7AKUTGUNn86CUe1lshGof\n"
    "sFxiOHUgOqGcWEC24G18rgt1vCKaxWDfTGO3pY+55bzFpnVdnBlK7kPG2+j+Bj8W\n"
    "NwUEwQIDAQABoxUwEzARBgNVHREECjAIggZzZXJ2ZXIwDQYJKoZIhvcNAQELBQAD\n"
    "ggEBAKawb0cI0Ieu19mk5LFkK74Hw7s4+58SucdC93zNBvitm1fnMDmCpYfFTsEV\n"
    "LuuheCQGQSCwL5IMDXdhaHMefL1VTvkws/QOQWYcHB3M3SvC+nApkiSy7Z77+Q1i\n"
    "oetv9zLnPX1l9wV2GozxygbLrFAICBNbfCBoES06ZrAVI5WKpRyH1iBN0O2oky8g\n"
    "J6zWLo7/3QdkDXqxjPzAhe0QiLZj3gVbmb0qCDOQOb59WxfoXqH+0LYYWcz046p7\n"
    "Qc0UmyO6jI4gPtR+E3C/U7qHpD0fkz2aVJp+vleOBtzinGfAhYPDgTCwQ55xGU72\n"
    "MPZDaUDgJLPG+bgt0iK1AV7EBec=\n"
    "-----END CERTIFICATE-----\n"};

static const std::string kCertWithNoCNButWithSANKey{
    "-----BEGIN PRIVATE KEY-----\n"
    "MIIEwAIBADANBgkqhkiG9w0BAQEFAASCBKowggSmAgEAAoIBAQDWZEdsJ4zGnCz6\n"
    "wcI4UAAtmmbQuZ7praua0BAN02X5keP2MQ1o2nKqG2CYANDjF7h5D/dv70POnllm\n"
    "22pdC4nBKK1VCTOTcLx/AQXFtT2VeafZ5TTsRs2NzrKn5M218WJp7k1qK+6vJ4EY\n"
    "UBa1nsN8lbq7acPiIZwLPwMaJZDcDrrJmvYXu/LDDsvcgmBkkcu00QmrKo2n9Xrj\n"
    "fvJ79q/UxWm2/IcX/HUbYLmyDs6agvxKHZ4j0pIntw/wYrsApRMZQ2fzoJR7WWyE\n"
    "ah+wXGI4dSA6oZxYQLbgbXyuC3W8IprFYN9MY7elj7nlvMWmdV2cGUruQ8bb6P4G\n"
    "PxY3BQTBAgMBAAECggEBAJsCzGVVv0KG/zqbR6thpI9UeQxneY/pww7fawwkEjI9\n"
    "mr6RvulWMNvviYq95EqeBwJ5WeWz8Kn+8hMdiC0YP5TKrXCzg3gSZifJ/HtzzMA7\n"
    "wvIX+IjxtIPYtHISS+5GRmrjI1QlyaEZBg0nMxREY4G73NTO5xOkS2gSOlL4YGHK\n"
    "bNdzQpLyZj6Goosr0g1J+l2/EVKO+/xBvZv6Fj6im+PmAimb+xe9iul7QDQHlYtI\n"
    "T9J+2lvgzAof+uGYkMLwIO4R6xnjfd+Ey91rrQXgLfu1H63IT5qD0cOQ8sp+6XOW\n"
    "slX6n1UxCjGYAJeSXqqu+5fX03XZt4h2zej2xtwLzeUCgYEA/RcnrLjzW2K1fnZq\n"
    "WYuLcl9YwG4gUqqAzPsVOBPsWpnU1DMuC/L24u/CIlGLEEPRXgt8r6nGqfSg6Hs4\n"
    "EOve8px3So5Nvt8EhugbANuQJlBPb3rkbV6cnCAbzpS6KYwCMNRZImO2K1bsab17\n"
    "GBNagzKy5wqp6TFY0+CMySxytJ8CgYEA2Ns70YY3iYDFLLxrVpxHQG9bR4chATE2\n"
    "tnePihfGgaW0oa744cMhJF/JIfpepB2JG/aOD6vmgtDO/z+vMqkO9Dvt/6wHa+S4\n"
    "5AVkJnIkApNWXYdPM53szLx9fiuiQrQszsVpBSA20ytG9JjZ8/8i4T80KTgV0Il+\n"
    "BrfzYYzjap8CgYEApCg8p59e2UtxBRGxcVs9m3WUj1vewz+sQ0goPzNM/ocAsJJx\n"
    "r3ZsBE4W0UOqu2YBispQmW+5V8tAAwrJFtCmzx4FkeozKzZkLUynFytSmEdG/rvr\n"
    "JbVURz/cSWXWSdRyJ1HUbqXWJs4+kWdBTCBheO+NcqZMBuDnCKaBKosV62kCgYEA\n"
    "mvx9ES+Cy9Rrl6CGep0w012C+GkcbpS0zM5AmWqKpig/I/tAx2HFcxC+WHlvYI33\n"
    "azPYrlymX8JK3uSuG1/2Xxnh7IQPvc33UoiD3nJfSDPzWt8U/QgWsPDaI+2dh1zs\n"
    "VU+D0nUGmf/pM3F2/ErRq/iXGAqMlMFff//Cg2rFMnMCgYEA7De3L3aNpHFWhtUi\n"
    "7c3DGtP3qTC6FAS9GJlA1TVsjgHt4nL3ZAKWOqXwVa1kXNDeYgN/Br39dfYW/KRE\n"
    "Liz9kBryc02/2YSjg4YFDgrCqzeERPmwj8AKM3sPyL+nLzZOvJ3EbhyiQiOYGh+U\n"
    "osLtL9tFEp8IrKHnoNVDuMPaZPM=\n"
    "-----END PRIVATE KEY-----\n"};

class SSLContextManagerForTest : public SSLContextManager {
 public:
  using SSLContextManager::addServerContext;
  using SSLContextManager::insertSSLCtxByDomainName;
  using SSLContextManager::SSLContextManager;
};

TEST(SSLContextManagerTest, Test1) {
  SSLContextManagerForTest sslCtxMgr(
      "vip_ssl_context_manager_test_", true, nullptr);
  auto www_example_com_ctx = std::make_shared<SSLContext>();
  auto start_example_com_ctx = std::make_shared<SSLContext>();
  auto start_abc_example_com_ctx = std::make_shared<SSLContext>();
  auto www_example_com_ctx_sha1 = std::make_shared<SSLContext>();
  auto start_example_com_ctx_sha1 = std::make_shared<SSLContext>();
  auto www_example_org_ctx_sha1 = std::make_shared<SSLContext>();

  sslCtxMgr.insertSSLCtxByDomainName(
      "*.example.com", start_example_com_ctx_sha1, CertCrypto::SHA1_SIGNATURE);
  sslCtxMgr.insertSSLCtxByDomainName("www.example.com", www_example_com_ctx);
  sslCtxMgr.insertSSLCtxByDomainName("www.example.com", www_example_com_ctx);
  sslCtxMgr.insertSSLCtxByDomainName("*.example.com", start_example_com_ctx);
  sslCtxMgr.insertSSLCtxByDomainName(
      "*.abc.example.com", start_abc_example_com_ctx);
  sslCtxMgr.insertSSLCtxByDomainName(
      "www.example.com", www_example_com_ctx_sha1, CertCrypto::SHA1_SIGNATURE);
  sslCtxMgr.insertSSLCtxByDomainName(
      "www.example.org", www_example_org_ctx_sha1, CertCrypto::SHA1_SIGNATURE);

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

  retCtx = sslCtxMgr.getSSLCtxByExactDomain(
      SSLContextKey("www.example.com", CertCrypto::SHA1_SIGNATURE));
  EXPECT_EQ(retCtx, www_example_com_ctx_sha1);
  retCtx = sslCtxMgr.getSSLCtxBySuffix(
      SSLContextKey("abc.example.com", CertCrypto::SHA1_SIGNATURE));
  EXPECT_EQ(retCtx, start_example_com_ctx_sha1);
  retCtx = sslCtxMgr.getSSLCtxBySuffix(
      SSLContextKey("xyz.abc.example.com", CertCrypto::SHA1_SIGNATURE));
  EXPECT_FALSE(retCtx);

  retCtx = sslCtxMgr.getSSLCtxByExactDomain(
      SSLContextKey("www.example.org", CertCrypto::SHA1_SIGNATURE));
  EXPECT_EQ(retCtx, www_example_org_ctx_sha1);
  retCtx = sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("www.example.org"));
  EXPECT_EQ(retCtx, www_example_org_ctx_sha1);
}

// This test uses multiple contexts, which requires SNI support to work at all.
#if FOLLY_OPENSSL_HAS_SNI
TEST(SSLContextManagerTest, TestResetSSLContextConfigs) {
  SSLContextManagerForTest sslCtxMgr(
      "vip_ssl_context_manager_test_", true, nullptr);
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
      {ctxConfig1Default}, cacheOptions, &seeds1, addr, nullptr);
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
      {ctxConfig2Default, ctxConfig3}, cacheOptions, nullptr, addr, nullptr);
  EXPECT_FALSE(sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test.com")));
  EXPECT_TRUE(sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test2.com")));
  EXPECT_TRUE(sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test3.com")));
  checkSeeds(
      sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test2.com")), seeds1);
  checkSeeds(
      sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test3.com")), seeds1);

  // New set of contexts, new seeds.
  sslCtxMgr.resetSSLContextConfigs(
      {ctxConfig1Default, ctxConfig3}, cacheOptions, &seeds2, addr, nullptr);
  EXPECT_TRUE(sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test.com")));
  EXPECT_FALSE(sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test2.com")));
  EXPECT_TRUE(sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test3.com")));
  checkSeeds(
      sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test.com")), seeds2);
  checkSeeds(
      sslCtxMgr.getSSLCtxByExactDomain(SSLContextKey("test3.com")), seeds2);

  // Back to one context, no new seeds.
  sslCtxMgr.resetSSLContextConfigs(
      {ctxConfig1Default}, cacheOptions, nullptr, addr, nullptr);
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
      "vip_ssl_context_manager_test_", true, nullptr);
  SSLContextConfig ctxConfig;
  ctxConfig.sessionContext = "test";
  ctxConfig.addCertificateBuf(kTestCert1PEM, kTestCert1Key);

  SSLCacheOptions cacheOptions;
  SocketAddress addr;

  sslCtxMgr.addSSLContextConfig(
      ctxConfig, cacheOptions, nullptr, addr, nullptr);

  SSLContextKey key("test.com", CertCrypto::BEST_AVAILABLE);
  auto ctx = sslCtxMgr.getSSLCtx(key);
  ASSERT_NE(ctx, nullptr);
  auto sessCtxFromCtx = std::string(
      reinterpret_cast<char*>(ctx->getSSLCtx()->sid_ctx),
      ctx->getSSLCtx()->sid_ctx_length);
  EXPECT_EQ(*ctxConfig.sessionContext, sessCtxFromCtx);
}

TEST(SSLContextManagerTest, TestSessionContextIfSessionCacheAbsent) {
  SSLContextManagerForTest sslCtxMgr(
      "vip_ssl_context_manager_test_", true, nullptr);
  SSLContextConfig ctxConfig;
  ctxConfig.sessionContext = "test";
  ctxConfig.sessionCacheEnabled = false;
  ctxConfig.addCertificateBuf(kTestCert1PEM, kTestCert1Key);

  SSLCacheOptions cacheOptions;
  SocketAddress addr;

  sslCtxMgr.addSSLContextConfig(
      ctxConfig, cacheOptions, nullptr, addr, nullptr);

  SSLContextKey key("test.com", CertCrypto::BEST_AVAILABLE);
  auto ctx = sslCtxMgr.getSSLCtx(key);
  ASSERT_NE(ctx, nullptr);
  auto sessCtxFromCtx = std::string(
      reinterpret_cast<char*>(ctx->getSSLCtx()->sid_ctx),
      ctx->getSSLCtx()->sid_ctx_length);
  EXPECT_EQ(*ctxConfig.sessionContext, sessCtxFromCtx);
}
#endif

TEST(SSLContextManagerTest, TestSessionContextCertRemoval) {
  SSLContextManagerForTest sslCtxMgr(
      "vip_ssl_context_manager_test_", true, nullptr);
  auto www_example_com_ctx = std::make_shared<ServerSSLContext>();
  auto start_example_com_ctx = std::make_shared<ServerSSLContext>();
  auto start_abc_example_com_ctx = std::make_shared<ServerSSLContext>();
  auto www_abc_example_com_ctx = std::make_shared<ServerSSLContext>();

  sslCtxMgr.insertSSLCtxByDomainName("www.example.com", www_example_com_ctx);
  sslCtxMgr.addServerContext(www_example_com_ctx);
  sslCtxMgr.insertSSLCtxByDomainName("*.example.com", start_example_com_ctx);
  sslCtxMgr.addServerContext(start_example_com_ctx);
  sslCtxMgr.insertSSLCtxByDomainName(
      "*.abc.example.com", start_abc_example_com_ctx);
  sslCtxMgr.addServerContext(start_abc_example_com_ctx);

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
  retCtx = sslCtxMgr.getSSLCtx(SSLContextKey("foo.example.com"));
  EXPECT_FALSE(retCtx);

  // Add it back and delete again but with the other API.
  sslCtxMgr.insertSSLCtxByDomainName("*.example.com", start_example_com_ctx);
  sslCtxMgr.addServerContext(start_example_com_ctx);
  retCtx = sslCtxMgr.getSSLCtx(SSLContextKey("foo.example.com"));
  EXPECT_TRUE(retCtx);
  sslCtxMgr.removeSSLContextConfigByDomainName("*.example.com");
  retCtx = sslCtxMgr.getSSLCtx(SSLContextKey("foo.example.com"));
  EXPECT_FALSE(retCtx);

  // Try to remove the context which does not exist - must be NOOP
  sslCtxMgr.removeSSLContextConfig(SSLContextKey("xyz.example.com"));

  // Setting a default context
  sslCtxMgr.insertSSLCtxByDomainName(
      "www.abc.example.com",
      www_abc_example_com_ctx,
      CertCrypto::BEST_AVAILABLE,
      true);

  // Context Manager must throw on attempt to remove the default context
  EXPECT_THROW(
      sslCtxMgr.removeSSLContextConfig(SSLContextKey("www.abc.example.com")),
      std::invalid_argument);
}

TEST(SSLContextManagerTest, TestCertificateWithNoCN) {
  SSLContextManagerForTest sslCtxMgr(
      "vip_ssl_context_manager_test_", true, nullptr);
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
  SSLContextKey key("O = interop runner", CertCrypto::BEST_AVAILABLE);
  auto ctx = sslCtxMgr.getSSLCtx(key);
  ASSERT_NE(ctx, nullptr);
}

TEST(SSLContextManagerTest, TestAlpnAllowMismatch) {
  SSLContextManagerForTest sslCtxMgr(
      "vip_ssl_context_manager_test_", true, nullptr);
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
  SSLContextKey key("O = interop runner", CertCrypto::BEST_AVAILABLE);
  auto ctx = sslCtxMgr.getSSLCtx(key);
  ASSERT_NE(ctx, nullptr);
  ASSERT_TRUE(ctx->getAlpnAllowMismatch());
}

TEST(SSLContextManagerTest, TestAlpnNotAllowMismatch) {
  SSLContextManagerForTest sslCtxMgr(
      "vip_ssl_context_manager_test_", true, nullptr);
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
  SSLContextKey key("O = interop runner", CertCrypto::BEST_AVAILABLE);
  auto ctx = sslCtxMgr.getSSLCtx(key);
  ASSERT_NE(ctx, nullptr);
  ASSERT_FALSE(ctx->getAlpnAllowMismatch());
}

TEST(SSLContextManagerTest, TestSingleClientCAFileSet) {
  SSLContextManagerForTest sslCtxMgr(
      "vip_ssl_context_manager_test_", true, nullptr);
  const std::string clientCAFile = "folly/io/async/test/certs/client_chain.pem";

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
      "vip_ssl_context_manager_test_", true, nullptr);
  const std::vector<std::string> clientCAFiles{
      "folly/io/async/test/certs/client_cert.pem",
      "folly/io/async/test/certs/tests-cert.pem"};

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

} // namespace wangle
