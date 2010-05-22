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

#include <test/test_ext_soap.h>
#include <runtime/ext/ext_output.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtSoap::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_SoapServerSanity);
  RUN_TEST(test_SoapServerFunctionAll);
  RUN_TEST(test_SoapServerFunctionParam);
  RUN_TEST(test_SoapServerArrayParam);
  RUN_TEST(test_SoapServerWSDL);
  RUN_TEST(test_SoapServerFault);

  return ret;
}

bool TestExtSoap::preTest() {
  m_server = sp_soapserver(NEW(c_soapserver)());
  m_server->create(null, CREATE_MAP1("uri", "http://testuri.org"));
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// helpers

bool TestExtSoap::verify_response(CStrRef request, CStrRef expected) {
  f_ob_start();
  m_server->t_handle(request);
  String res = f_ob_get_contents();
  f_ob_end_clean();
  VS(res, expected);
  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtSoap::test_SoapServerSanity() {
  m_server->t_addfunction("hello");

  VSOAP("<ns1:hello xmlns:ns1=\"http://testuri.org\" />",
        "<ns1:helloResponse><return xsi:type=\"xsd:string\">Hello World"
        "</return></ns1:helloResponse>");

  return Count(true);
}

bool TestExtSoap::test_SoapServerFunctionAll() {
  m_server->t_addfunction(k_SOAP_FUNCTIONS_ALL);

  VSOAP("<ns1:strlen xmlns:ns1=\"http://testuri.org\">"
        "<x xsi:type=\"xsd:string\">Hello World</x>"
        "</ns1:strlen>",
        "<ns1:strlenResponse><return xsi:type=\"xsd:int\">11"
        "</return></ns1:strlenResponse>");

  return Count(true);
}

bool TestExtSoap::test_SoapServerFunctionParam() {
  Array funcs = CREATE_VECTOR2("Sub", "Add");
  m_server->t_addfunction(funcs);
  VS(m_server->t_getfunctions(), funcs);

  VSOAP("<ns1:Add xmlns:ns1=\"http://testuri.org\">"
        "<x xsi:type=\"xsd:int\">22</x>"
        "<y xsi:type=\"xsd:int\">33</y>"
        "</ns1:Add>",
        "<ns1:AddResponse><return xsi:type=\"xsd:int\">55"
        "</return></ns1:AddResponse>");

  return Count(true);
}

bool TestExtSoap::test_SoapServerArrayParam() {
  m_server->t_addfunction("Sum");

  VSOAP("<ns1:sum>"
        "<param0 SOAP-ENC:arrayType=\"xsd:int[2]\""
        " xsi:type=\"SOAP-ENC:Array\">"
        "  <val xsi:type=\"xsd:int\">3</val>"
        "  <val xsi:type=\"xsd:int\">5</val>"
        "</param0>"
        "</ns1:sum>",
        "<ns1:sumResponse><return xsi:type=\"xsd:int\">8"
        "</return></ns1:sumResponse>");

  return Count(true);
}

bool TestExtSoap::test_SoapServerWSDL() {
  m_server = sp_soapserver(NEW(c_soapserver)());
  m_server->create("test/test.wsdl", CREATE_MAP1("uri", "http://testuri.org"));
  m_server->t_addfunction("Add");

  VSOAPNS("<ns1:Add xmlns:ns1=\"http://testuri.org\">"
          "<x xsi:type=\"xsd:int\">22</x>"
          "<y xsi:type=\"xsd:int\">33</y>"
          "</ns1:Add>",
          "<ns1:AddResponse><result xsi:type=\"xsd:double\">55"
          "</result></ns1:AddResponse>",
          "");

  return Count(true);
}

bool TestExtSoap::test_SoapServerFault() {
  m_server->t_addfunction("Fault");

  VSOAPEX("<ns1:fault xmlns:ns1=\"http://testuri.org\"/>",
          "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
          "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\"><SOAP-ENV:Body><SOAP-ENV:Fault><faultcode>MyFault</faultcode><faultstring>My fault string</faultstring></SOAP-ENV:Fault></SOAP-ENV:Body></SOAP-ENV:Envelope>\n");

  return Count(true);
}
