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

#include <test/test_ext_domdocument.h>
#include <runtime/ext/ext_domdocument.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtDomdocument::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_DOMNode);
  RUN_TEST(test_DOMAttr);
  RUN_TEST(test_DOMCharacterData);
  RUN_TEST(test_DOMComment);
  RUN_TEST(test_DOMText);
  RUN_TEST(test_DOMCdataSection);
  RUN_TEST(test_DOMDocument);
  RUN_TEST(test_DOMDocumentFragment);
  RUN_TEST(test_DOMDocumentType);
  RUN_TEST(test_DOMElement);
  RUN_TEST(test_DOMEntity);
  RUN_TEST(test_DOMEntityReference);
  RUN_TEST(test_DOMNotation);
  RUN_TEST(test_DOMProcessingInstruction);
  RUN_TEST(test_DOMNamedNodeMap);
  RUN_TEST(test_DOMNodeList);
  RUN_TEST(test_DOMException);
  RUN_TEST(test_DOMImplementation);
  RUN_TEST(test_DOMXPath);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtDomdocument::test_DOMNode() {
  return Count(true);
}

bool TestExtDomdocument::test_DOMAttr() {
  return Count(true);
}

bool TestExtDomdocument::test_DOMCharacterData() {
  return Count(true);
}

bool TestExtDomdocument::test_DOMComment() {
  return Count(true);
}

bool TestExtDomdocument::test_DOMText() {
  return Count(true);
}

bool TestExtDomdocument::test_DOMCdataSection() {
  return Count(true);
}

bool TestExtDomdocument::test_DOMDocument() {
  return Count(true);
}

bool TestExtDomdocument::test_DOMDocumentFragment() {
  return Count(true);
}

bool TestExtDomdocument::test_DOMDocumentType() {
  return Count(true);
}

bool TestExtDomdocument::test_DOMElement() {
  return Count(true);
}

bool TestExtDomdocument::test_DOMEntity() {
  return Count(true);
}

bool TestExtDomdocument::test_DOMEntityReference() {
  return Count(true);
}

bool TestExtDomdocument::test_DOMNotation() {
  return Count(true);
}

bool TestExtDomdocument::test_DOMProcessingInstruction() {
  return Count(true);
}

bool TestExtDomdocument::test_DOMNamedNodeMap() {
  return Count(true);
}

bool TestExtDomdocument::test_DOMNodeList() {
  return Count(true);
}

bool TestExtDomdocument::test_DOMException() {
  return Count(true);
}

bool TestExtDomdocument::test_DOMImplementation() {
  return Count(true);
}

bool TestExtDomdocument::test_DOMXPath() {
  return Count(true);
}
