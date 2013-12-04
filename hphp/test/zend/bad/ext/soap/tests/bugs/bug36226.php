<?php
ini_set("soap.wsdl_cache_enabled",0);
$timestamp = "2005-11-08T11:22:07+03:00";
$wsdl = dirname(__FILE__)."/bug35142.wsdl";

function PostEvents($x) {
	var_dump($x);
	exit();
  return $x;
}

class TestSoapClient extends SoapClient {

  function __construct($wsdl, $options) {
    parent::__construct($wsdl, $options);
    $this->server = new SoapServer($wsdl, $options);
    $this->server->addFunction('PostEvents');
  }

  function __doRequest($request, $location, $action, $version, $one_way = 0) {
		echo "$request\n";
    $this->server->handle($request);
    return $response;
  }

}

$soapClient = new TestSoapClient($wsdl, 
	array('trace' => 1, 'exceptions' => 0,
		'classmap' => array('logOnEvent' => 'LogOnEvent',
			'logOffEvent' => 'LogOffEvent',
			'events' => 'IVREvents'),
		'features' => SOAP_SINGLE_ELEMENT_ARRAYS));

$logOnEvent = new LogOnEvent(34567, $timestamp);
$logOffEvents[] = new LogOffEvent(34567, $timestamp, "Smoked");
$logOffEvents[] = new LogOffEvent(34568, $timestamp, "SmokeFree");
$ivrEvents = new IVREvents("1.0", 101, 12345, 'IVR', $logOnEvent, $logOffEvents);

$result = $soapClient->PostEvents($ivrEvents);

class LogOffEvent {
  public $audienceMemberId;
  public $timestamp;
  public $smokeStatus;
  public $callInitiator;

  function __construct($audienceMemberId, $timestamp, $smokeStatus) {
    $this->audienceMemberId = $audienceMemberId;
    $this->timestamp = $timestamp;
    $this->smokeStatus = $smokeStatus;
    $this->callInitiator = "IVR";
  }
}

class LogOnEvent {
  public $audienceMemberId;
  public $timestamp;

  function __construct($audienceMemberId, $timestamp) {
    $this->audienceMemberId = $audienceMemberId;
    $this->timestamp = $timestamp;
  }
}

class IVREvents {
  public $version;
  public $activityId;
  public $messageId;
  public $source;
  public $logOnEvent;
  public $logOffEvent;

  function __construct($version, $activityId, $messageId, $source, $logOnEvent=NULL, $logOffEvent=NULL) {
    $this->version = $version;
    $this->activityId = $activityId;
    $this->messageId = $messageId;
    $this->source = $source;
    $this->logOnEvent = $logOnEvent;
    $this->logOffEvent = $logOffEvent;
  }
}
?>