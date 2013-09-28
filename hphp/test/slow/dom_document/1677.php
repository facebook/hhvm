<?php

function rerender($html, $frag = false) {
    $doc = new DOMDocument();
    if ($frag) {
      $body = $doc->createDocumentFragment();
      $body->appendXML($html);
    }
 else {
      $doc->loadHTML($html);
      $body = $doc->documentElement;
    }
    return helper($body);
  }
  function helper($element) {
    if ($element instanceof DOMText) {
      return htmlspecialchars($element->nodeValue);
    }
 else {
      $body = '';
      foreach ($element->childNodes as $child) {
        $body .= helper($child);
      }
      if ($element instanceof DOMElement) {
        $attrs = array();
        foreach ($element->attributes as $attr) {
          $attrs[] = htmlspecialchars($attr->name) . '="' .             htmlspecialchars($attr->value) . '"';
        }
        if ($attrs) {
          $attrs = ' ' . implode(' ', $attrs);
        }
 else {
          $attrs = '';
        }
        return '<' . $element->tagName . $attrs . '>' . $body .           '</' . $element->tagName . '>';
      }
 else {
        return $body;
      }
    }
  }
  $fragment = 'Hello, <b>world</b>.';
  $document = '<html><body><div style="color:red">    <p class="thing">'.$fragment.'</p></div>';
  echo rerender($fragment, true)."

";
  echo rerender($document, false)."

";
