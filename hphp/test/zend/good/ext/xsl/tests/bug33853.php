<?hh

<<__EntryPoint>> function main(): void {
$xsl = new DOMDocument();
$xsl->loadXML('<?xml version="1.0" encoding="iso-8859-1" ?>
<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns:php="http://php.net/xsl">
<xsl:template match="/">
<xsl:value-of select="php:function(\'TeSt::dateLang\')" />
</xsl:template>
</xsl:stylesheet>');
$inputdom = new DOMDocument();
$inputdom->loadXML('<?xml version="1.0" encoding="iso-8859-1" ?>
<today></today>');

$proc = new XSLTProcessor();
$proc->registerPHPFunctions();
$xsl = $proc->importStylesheet($xsl);
$newdom = $proc->transformToDoc($inputdom);
echo "===DONE===\n";
}
