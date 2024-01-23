<?hh


// This is a modified version of zend test DOMDocument_saveHTML_error2.php.
// Since parameter type check is performed before detecting invocation of
// non-static method from a static context, the error message differs.

<<__EntryPoint>>
function main_dom_document_save_htm_l_error2() :mixed{
  (new DOMDocument())->saveHTML(true);
}
