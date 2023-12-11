<?hh

// This example is from https://github.com/facebook/hhvm/issues/3440
<<__EntryPoint>>
function main_C14N() :mixed{
$xml = '<?xml version="1.0"?>' . "\n"
. '<samlp:Response xmlns:saml="urn:oasis:names:tc:SAML:2.0:assertion" xmlns:samlp="urn:oasis:names:tc:SAML:2.0:protocol" xm'
. 'lns:xs="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" Destination="https://pro'
. 'ject.mysite.com/saml/acs" ID="FIMRSP_5990c800-0147-1411-b087-93da9d684e69" IssueInstant="2014-07-21T15:36:06Z" Ver'
. 'sion="2.0"><saml:Issuer Format="urn:oasis:names:tc:SAML:2.0:nameid-format:entity">https://accessuat.mysite.com/fim/sps'
. '/saml20/saml20</saml:Issuer><samlp:Status><samlp:StatusCode Value="urn:oasis:names:tc:SAML:2.0:status:Success"/></samlp:'
. 'Status><saml:Assertion ID="Assertion-uuid5990c7c6-0147-19bd-9537-93da9d684e69" IssueInstant="2014-07-21T15:36:06Z" Versi'
. 'on="2.0"><saml:Issuer Format="urn:oasis:names:tc:SAML:2.0:nameid-format:entity">https://accessuat.mysite.com/fim/sps/s'
. 'aml20/saml20</saml:Issuer><saml:Subject><saml:NameID Format="urn:oasis:names:tc:SAML:1.'
. '1:nameid-format:unspecified">testjive</saml:NameID><saml:SubjectConfirmation Method="urn:oasis:names:tc:SAML:2.0:cm:bear'
. 'er"><saml:SubjectConfirmationData NotOnOrAfter="2014-07-21T15:46:06Z" Recipient="https://project.mysite.com/saml/a'
. 'cs"/></saml:SubjectConfirmation></saml:Subject><saml:Conditions NotBefore="2014-07-21T15:26:06Z" NotOnOrAfter="2014-07-2'
. '1T15:46:06Z"><saml:AudienceRestriction><saml:Audience>http://project.mysite.com</saml:Audience></saml:AudienceRest'
. 'riction></saml:Conditions><saml:AuthnStatement AuthnInstant="2014-07-21T15:36:06Z" SessionIndex="uuid599090e7-0147-15b6-'
. '9dfd-93da9d684e69" SessionNotOnOrAfter="2014-07-21T16:36:06Z"><saml:AuthnContext><saml:AuthnContextClassRef>urn:oasis:na'
. 'mes:tc:SAML:2.0:ac:classes:Password</saml:AuthnContextClassRef></saml:AuthnContext></saml:AuthnStatement><saml:Attribute'
. 'Statement><saml:Attribute Name="LastName" NameFormat="urn:oasis:names:tc:SAML:1.1:nameid-format:unspecified"><saml:Attri'
. 'buteValue xsi:type="xs:string">user</saml:AttributeValue></saml:Attribute><saml:Attribute Name="FirstName" NameFormat="u'
. 'rn:oasis:names:tc:SAML:1.1:nameid-format:unspecified"><saml:AttributeValue xsi:type="xs:string">Test</saml:AttributeValu'
. 'e></saml:Attribute><saml:Attribute Name="email" NameFormat="urn:oasis:names:tc:SAML:1.1:nameid-format:unspecified"><saml'
. ':AttributeValue xsi:type="xs:string">idam_support@bp.com</saml:AttributeValue></saml:Attribute><saml:Attribute Name="use'
. 'r-id" NameFormat="urn:oasis:names:tc:SAML:1.1:nameid-format:unspecified"><saml:AttributeValue xsi:type="xs:string">testj'
. 'ive</saml:AttributeValue></saml:Attribute></saml:AttributeStatement></saml:Assertion></samlp:Response>';


$doc = new DOMDocument();
$res = $doc->loadXML($xml);

$caNode = $doc->C14N(true, false, null, vec['xs', 'xsi', 'saml']);
echo md5($caNode);
}
