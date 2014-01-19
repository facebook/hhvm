<?php

$xmlstr = "<?xml version='1.0' standalone='yes'?>
<!DOCTYPE chapter SYSTEM '/share/sgml/Norman_Walsh/db3xml10/db3xml10.dtd'
[ <!ENTITY sp \"spanish\">
]>
<!-- lsfj  -->
<chapter language='en'><title language='en'>Title</title>
<para language='ge'>
&sp;
<!-- comment -->
<informaltable language='&sp;kkk'>
<tgroup cols='3'>
<tbody>
<row><entry>a1</entry><entry morerows='1'>b1</entry><entry>c1</entry></row>
<row><entry>a2</entry><entry>c2</entry></row>
<row><entry>a3</entry><entry>b3</entry><entry>c3</entry></row>
</tbody>
</tgroup>
</informaltable>
</para>
</chapter> ";

function print_node($node)
{
  print "Node Name: " . $node->nodeName;
  print "
Node Type: " . $node->nodeType;
  if ($node->nodeType != 3) {
      $child_count = $node->childNodes->length;
  }
 else {
      $child_count = 0;
  }
  print "
Num Children: " . $child_count;
  if($child_count <= 1){
    print "
Node Content: " . $node->nodeValue;
  }
  print "

";
}

function print_node_list($nodelist)
{
  foreach($nodelist as $node)
  {
    print_node($node);
  }
}

echo "Test 1: accessing single nodes from php
";
$dom = new domDocument;
$dom->loadxml($xmlstr);
if(!$dom) {
  echo "Error while parsing the document
";
  exit;
}

// children() of of document would result in a memleak
//$children = $dom->children();
//print_node_list($children);

echo "--------- root
";
$rootnode = $dom->documentElement;
print_node($rootnode);

echo "--------- children of root
";
$children = $rootnode->childNodes;
print_node_list($children);

// The last node should be identical with the last entry in the children array
echo "--------- last
";
$last = $rootnode->lastChild;
print_node($last);

// The parent of this last node is the root again
echo "--------- parent
";
$parent = $last->parentNode;
print_node($parent);

// The children of this parent are the same children as one above
echo "--------- children of parent
";
$children = $parent->childNodes;
print_node_list($children);

echo "--------- creating a new attribute
";
//This is worthless
//$attr = $dom->createAttribute("src", "picture.gif");
//print_r($attr);

//$rootnode->set_attributeNode($attr);
$attr = $rootnode->setAttribute("src", "picture.gif");
$attr = $rootnode->getAttribute("src");
print_r($attr);
print "
";

echo "--------- Get Attribute Node
";
$attr = $rootnode->getAttributeNode("src");
print_node($attr);

echo "--------- Remove Attribute Node
";
$attr = $rootnode->removeAttribute("src");
print "Removed " . $attr . " attributes.
";

echo "--------- attributes of rootnode
";
$attrs = $rootnode->attributes;
print_node_list($attrs);

echo "--------- children of an attribute
";
$children = $attrs->item(0)->childNodes;
print_node_list($children);

echo "--------- Add child to root
";
$myelement = new domElement("Silly", "Symphony");
$newchild = $rootnode->appendChild($myelement);
print_node($newchild);
print $dom->saveXML();
print "
";

echo "--------- Find element by tagname
";
echo "    Using dom
";
$children = $dom->getElementsByTagname("Silly");
print_node_list($children);

echo "    Using elem
";
$children = $rootnode->getElementsByTagName("Silly");
print_node_list($children);

echo "--------- Unlink Node
";
print_node($children->item(0));
$rootnode->removeChild($children->item(0));
print_node_list($rootnode->childNodes);
print $dom->savexml();
