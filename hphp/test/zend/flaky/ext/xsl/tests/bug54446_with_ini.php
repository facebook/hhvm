<?php
include("prepare.inc"); 

$outputfile = dirname(__FILE__)."/bug54446test.txt";
if (file_exists($outputfile)) {
    unlink($outputfile);
}

$sXsl = <<<EOT
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:sax="http://icl.com/saxon"
	extension-element-prefixes="sax">

	<xsl:template match="/">
		<sax:output href="$outputfile" method="text">
			<xsl:value-of select="'0wn3d via PHP and libxslt ...'"/>
		</sax:output>
	</xsl:template>

</xsl:stylesheet>
EOT;

$xsl->loadXML( $sXsl );

# START XSLT 
$proc->importStylesheet( $xsl ); 

# TRASNFORM & PRINT 
print $proc->transformToXML( $dom ); 


if (file_exists($outputfile)) {
    print "$outputfile exists, but shouldn't!\n";
} else {
    print "OK, no file created\n";
}

#SET NO SECURITY PREFS
ini_set("xsl.security_prefs", XSL_SECPREF_NONE);

# TRASNFORM & PRINT 
print $proc->transformToXML( $dom ); 


if (file_exists($outputfile)) {
    print "OK, file exists\n";
} else {
    print "$outputfile doesn't exist, but should!\n";
}

unlink($outputfile);

#SET SECURITY PREFS AGAIN
ini_set("xsl.security_prefs", XSL_SECPREF_WRITE_FILE |  XSL_SECPREF_WRITE_NETWORK | XSL_SECPREF_CREATE_DIRECTORY);

# TRASNFORM & PRINT 
print $proc->transformToXML( $dom ); 

if (file_exists($outputfile)) {
    print "$outputfile exists, but shouldn't!\n";
} else {
    print "OK, no file created\n";
}

#SET NO SECURITY PREFS with ini, but set them with ->setSecurityPrefs
ini_set("xsl.security_prefs", XSL_SECPREF_NONE);
$proc->setSecurityPrefs( XSL_SECPREF_WRITE_FILE |  XSL_SECPREF_WRITE_NETWORK | XSL_SECPREF_CREATE_DIRECTORY);

print $proc->transformToXML( $dom ); 
if (file_exists($outputfile)) {
    print "$outputfile exists, but shouldn't!\n";
} else {
    print "OK, no file created\n";
}

#don't throw a warning if both ini and through-the-method have the same value
$proc->setSecurityPrefs(XSL_SECPREF_NONE);

print $proc->transformToXML( $dom ); 

if (file_exists($outputfile)) {
    print "OK, file exists\n";
} else {
    print "$outputfile doesn't exist, but should!\n";
}
unlink($outputfile);


