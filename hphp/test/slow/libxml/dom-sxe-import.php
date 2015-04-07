<?php
// c.f. https://github.com/facebook/hhvm/issues/2408

function load($data) {
  $element = new SimpleXMLElement($data);
  $fields  = $element->xpath('descendant-or-self::field');
  foreach ($fields as $field) {
    $loadeddom = dom_import_simplexml($field);
    $current   = simplexml_load_string('<books></books>');
    $olddom    = dom_import_simplexml($current);
    $addeddom  = $olddom->ownerDocument->importNode($loadeddom);
    $olddom->parentNode->replaceChild($addeddom, $olddom);
  }
}

$loadMergeDocument = '
   <fields>
      <field name="published" type="list">
         <option value="1">JYES</option>
         <option value="0">JNO</option>
      </field>
      <field name="abstract" label="Abstract" />
      <fields label="A general group">
         <field name="access" />
         <field name="ordering" />
      </fields>
      <fields name="params">
         <field name="show_abstract" type="radio">
            <option value="1">JYes</option>
            <option value="0">JNo</option>
         </field>
      </fields>
      <fieldset>
         <field name="language" type="text" />
      </fieldset>
   </fields>
';

load($loadMergeDocument);
echo "Done.\n";
