/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef __TEST_EXT_SOAP_H__
#define __TEST_EXT_SOAP_H__

#include <test/test_cpp_ext.h>
#include <runtime/ext/ext_soap.h>

///////////////////////////////////////////////////////////////////////////////

class TestExtSoap : public TestCppExt {
public:
  virtual bool preTest();
  virtual bool RunTests(const std::string &which);

  bool test_SoapServerSanity();
  bool test_SoapServerFunctionAll();
  bool test_SoapServerFunctionParam();
  bool test_SoapServerArrayParam();
  bool test_SoapServerWSDL();
  bool test_SoapServerFault();

private:
  sp_soapserver m_server;

  bool verify_response(CStrRef request, CStrRef expected);
};

#define VSOAPEX(request, response)                                      \
  if (!verify_response                                                  \
      ("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>"                \
       "<SOAP-ENV:Envelope SOAP-ENV:encodingStyle="                     \
       "\"http://schemas.xmlsoap.org/soap/encoding/\""                  \
       " xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\""  \
       " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""                \
       " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""       \
       " xmlns:si=\"http://soapinterop.org/xsd\">"                      \
       "<SOAP-ENV:Body>"                                                \
       request                                                          \
       "  </SOAP-ENV:Body>"                                             \
       "</SOAP-ENV:Envelope>",                                          \
       response                                                         \
       )) return false;

#define VSOAPNS(request, expected, ns)                                  \
  VSOAPEX(request,                                                      \
          "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"                \
          "<SOAP-ENV:Envelope xmlns:SOAP-ENV="                          \
          "\"http://schemas.xmlsoap.org/soap/envelope/\""               \
          " xmlns:ns1=\"" ns "\""                                       \
          " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""             \
          " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""    \
          " xmlns:SOAP-ENC="                                            \
          "\"http://schemas.xmlsoap.org/soap/encoding/\""               \
          " SOAP-ENV:encodingStyle="                                    \
          "\"http://schemas.xmlsoap.org/soap/encoding/\">"              \
          "<SOAP-ENV:Body>"                                             \
          expected                                                      \
          "</SOAP-ENV:Body></SOAP-ENV:Envelope>\n"                      \
          )

#define VSOAP(request, expected) \
  VSOAPNS(request, expected, "http://testuri.org")

///////////////////////////////////////////////////////////////////////////////

#endif // __TEST_EXT_SOAP_H__
