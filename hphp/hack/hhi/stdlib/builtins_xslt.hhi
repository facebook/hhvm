<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

class XSLTProcessor {
  public function getParameter(?string $namespaceURI, string $localName): mixed;
  public function getSecurityPrefs(): int;
  public function hasExsltSupport(): bool;
  public function importStylesheet(DOMDocument $stylesheet): void;
  public function registerPHPFunctions(mixed $funcs = null): void;
  public function removeParameter(
    ?string $namespaceURI,
    string $localName,
  ): bool;
  public function setParameter(
    ?string $namespaceURI,
    mixed $localName,
    mixed $value = null,
  ): bool;
  public function setProfiling(string $filename): bool;
  public function setSecurityPrefs(int $securityPrefs): int;
  public function transformToDoc(DOMNode $doc): mixed;
  public function transformToURI(DOMDocument $doc, string $uri): mixed;
  public function transformToXML(DOMDocument $doc): mixed;
}
