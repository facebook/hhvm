<?php
/*
 * This file is part of PHP-FastCGI-Client.
 *
 * (c) Pierrick Charron <pierrick@adoy.net>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */
namespace Adoy\FastCGI;

class TimedOutException extends \Exception {}
class ForbiddenException extends \Exception {}

/**
 * Handles communication with a FastCGI application
 *
 * @author      Pierrick Charron <pierrick@adoy.net>
 * @version     1.0
 */
class Client
{
    const VERSION_1            = 1;

    const BEGIN_REQUEST        = 1;
    const ABORT_REQUEST        = 2;
    const END_REQUEST          = 3;
    const PARAMS               = 4;
    const STDIN                = 5;
    const STDOUT               = 6;
    const STDERR               = 7;
    const DATA                 = 8;
    const GET_VALUES           = 9;
    const GET_VALUES_RESULT    = 10;
    const UNKNOWN_TYPE         = 11;
    const MAXTYPE              = self::UNKNOWN_TYPE;

    const RESPONDER            = 1;
    const AUTHORIZER           = 2;
    const FILTER               = 3;

    const REQUEST_COMPLETE     = 0;
    const CANT_MPX_CONN        = 1;
    const OVERLOADED           = 2;
    const UNKNOWN_ROLE         = 3;

    const MAX_CONNS            = 'MAX_CONNS';
    const MAX_REQS             = 'MAX_REQS';
    const MPXS_CONNS           = 'MPXS_CONNS';

    const HEADER_LEN           = 8;

    const REQ_STATE_WRITTEN    = 1;
    const REQ_STATE_OK         = 2;
    const REQ_STATE_ERR        = 3;
    const REQ_STATE_TIMED_OUT  = 4;

    /**
     * Socket
     * @var Resource
     */
    private $_sock = null;

    /**
     * Host
     * @var String
     */
    private $_host = null;

    /**
     * Port
     * @var Integer
     */
    private $_port = null;

    /**
     * Keep Alive
     * @var Boolean
     */
    private $_keepAlive = false;

    /**
     * Outstanding request statuses keyed by request id
     *
     * Each request is an array with following form:
     *
     *  array(
     *    'state' => REQ_STATE_*
     *    'response' => null | string
     *  )
     *
     * @var array
     */
    private $_requests = array();

    /**
     * Use persistent sockets to connect to backend
     * @var Boolean
     */
    private $_persistentSocket = false;

    /**
     * Connect timeout in milliseconds
     * @var Integer
     */
    private $_connectTimeout = 5000;

    /**
     * Read/Write timeout in milliseconds
     * @var Integer
     */
    private $_readWriteTimeout = 5000;

    /**
     * Constructor
     *
     * @param String $host Host of the FastCGI application
     * @param Integer $port Port of the FastCGI application
     */
    public function __construct($host, $port)
    {
        $this->_host = $host;
        $this->_port = $port;
    }

    /**
     * Define whether or not the FastCGI application should keep the connection
     * alive at the end of a request
     *
     * @param Boolean $b true if the connection should stay alive, false otherwise
     */
    public function setKeepAlive($b)
    {
        $this->_keepAlive = (boolean)$b;
        if (!$this->_keepAlive && $this->_sock) {
            fclose($this->_sock);
        }
    }

    /**
     * Get the keep alive status
     *
     * @return Boolean true if the connection should stay alive, false otherwise
     */
    public function getKeepAlive()
    {
        return $this->_keepAlive;
    }

    /**
     * Define whether or not PHP should attempt to re-use sockets opened by previous
     * request for efficiency
     *
     * @param Boolean $b true if persistent socket should be used, false otherwise
     */
    public function setPersistentSocket($b)
    {
        $was_persistent = ($this->_sock && $this->_persistentSocket);
        $this->_persistentSocket = (boolean)$b;
        if (!$this->_persistentSocket && $was_persistent) {
            fclose($this->_sock);
        }
    }

    /**
     * Get the pesistent socket status
     *
     * @return Boolean true if the socket should be persistent, false otherwise
     */
    public function getPersistentSocket()
    {
        return $this->_persistentSocket;
    }


    /**
     * Set the connect timeout
     *
     * @param Integer  number of milliseconds before connect will timeout
     */
    public function setConnectTimeout($timeoutMs)
    {
        $this->_connectTimeout = $timeoutMs;
    }

    /**
     * Get the connect timeout
     *
     * @return Integer  number of milliseconds before connect will timeout
     */
    public function getConnectTimeout()
    {
        return $this->_connectTimeout;
    }

    /**
     * Set the read/write timeout
     *
     * @param Integer  number of milliseconds before read or write call will timeout
     */
    public function setReadWriteTimeout($timeoutMs)
    {
        $this->_readWriteTimeout = $timeoutMs;
        $this->set_ms_timeout($this->_readWriteTimeout);
    }

    /**
     * Get the read timeout
     *
     * @return Integer  number of milliseconds before read will timeout
     */
    public function getReadWriteTimeout()
    {
        return $this->_readWriteTimeout;
    }

    /**
     * Helper to avoid duplicating milliseconds to secs/usecs in a few places
     *
     * @param Integer millisecond timeout
     * @return Boolean
     */
    private function set_ms_timeout($timeoutMs) {
        if (!$this->_sock) {
            return false;
        }
        return stream_set_timeout($this->_sock, floor($timeoutMs / 1000), ($timeoutMs % 1000) * 1000);
    }


    /**
     * Create a connection to the FastCGI application
     */
    private function connect()
    {
        if (!$this->_sock) {
            if ($this->_persistentSocket) {
                $this->_sock = pfsockopen($this->_host, $this->_port, $errno, $errstr, $this->_connectTimeout/1000);
            } else {
                $this->_sock = fsockopen($this->_host, $this->_port, $errno, $errstr, $this->_connectTimeout/1000);
            }

            if (!$this->_sock) {
                throw new \Exception('Unable to connect to FastCGI application: ' . $errstr);
            }

            if (!$this->set_ms_timeout($this->_readWriteTimeout)) {
                throw new \Exception('Unable to set timeout on socket');
            }
        }
    }

    /**
     * Build a FastCGI packet
     *
     * @param Integer $type Type of the packet
     * @param String $content Content of the packet
     * @param Integer $requestId RequestId
     */
    private function buildPacket($type, $content, $requestId = 1)
    {
        $clen = strlen($content);
        return chr(self::VERSION_1)         /* version */
            . chr($type)                    /* type */
            . chr(($requestId >> 8) & 0xFF) /* requestIdB1 */
            . chr($requestId & 0xFF)        /* requestIdB0 */
            . chr(($clen >> 8 ) & 0xFF)     /* contentLengthB1 */
            . chr($clen & 0xFF)             /* contentLengthB0 */
            . chr(0)                        /* paddingLength */
            . chr(0)                        /* reserved */
            . $content;                     /* content */
    }

    /**
     * Build an FastCGI Name value pair
     *
     * @param String $name Name
     * @param String $value Value
     * @return String FastCGI Name value pair
     */
    private function buildNvpair($name, $value)
    {
        $nlen = strlen($name);
        $vlen = strlen($value);
        if ($nlen < 128) {
            /* nameLengthB0 */
            $nvpair = chr($nlen);
        } else {
            /* nameLengthB3 & nameLengthB2 & nameLengthB1 & nameLengthB0 */
            $nvpair = chr(($nlen >> 24) | 0x80) . chr(($nlen >> 16) & 0xFF) . chr(($nlen >> 8) & 0xFF) . chr($nlen & 0xFF);
        }
        if ($vlen < 128) {
            /* valueLengthB0 */
            $nvpair .= chr($vlen);
        } else {
            /* valueLengthB3 & valueLengthB2 & valueLengthB1 & valueLengthB0 */
            $nvpair .= chr(($vlen >> 24) | 0x80) . chr(($vlen >> 16) & 0xFF) . chr(($vlen >> 8) & 0xFF) . chr($vlen & 0xFF);
        }
        /* nameData & valueData */
        return $nvpair . $name . $value;
    }

    /**
     * Read a set of FastCGI Name value pairs
     *
     * @param String $data Data containing the set of FastCGI NVPair
     * @return array of NVPair
     */
    private function readNvpair($data, $length = null)
    {
        $array = array();

        if ($length === null) {
            $length = strlen($data);
        }

        $p = 0;

        while ($p != $length) {

            $nlen = ord($data{$p++});
            if ($nlen >= 128) {
                $nlen = ($nlen & 0x7F << 24);
                $nlen |= (ord($data{$p++}) << 16);
                $nlen |= (ord($data{$p++}) << 8);
                $nlen |= (ord($data{$p++}));
            }
            $vlen = ord($data{$p++});
            if ($vlen >= 128) {
                $vlen = ($nlen & 0x7F << 24);
                $vlen |= (ord($data{$p++}) << 16);
                $vlen |= (ord($data{$p++}) << 8);
                $vlen |= (ord($data{$p++}));
            }
            $array[substr($data, $p, $nlen)] = substr($data, $p+$nlen, $vlen);
            $p += ($nlen + $vlen);
        }

        return $array;
    }

    /**
     * Decode a FastCGI Packet
     *
     * @param String $data String containing all the packet
     * @return array
     */
    private function decodePacketHeader($data)
    {
        $ret = array();
        $ret['version']       = ord($data{0});
        $ret['type']          = ord($data{1});
        $ret['requestId']     = (ord($data{2}) << 8) + ord($data{3});
        $ret['contentLength'] = (ord($data{4}) << 8) + ord($data{5});
        $ret['paddingLength'] = ord($data{6});
        $ret['reserved']      = ord($data{7});
        return $ret;
    }

    /**
     * Read a FastCGI Packet
     *
     * @return array
     */
    private function readPacket()
    {
        if ($packet = fread($this->_sock, self::HEADER_LEN)) {
            $resp = $this->decodePacketHeader($packet);
            $resp['content'] = '';
            if ($resp['contentLength']) {
                $len  = $resp['contentLength'];
                while ($len && $buf=fread($this->_sock, $len)) {
                    $len -= strlen($buf);
                    $resp['content'] .= $buf;
                }
            }
            if ($resp['paddingLength']) {
                $buf = fread($this->_sock, $resp['paddingLength']);
            }
            return $resp;
        } else {
            return false;
        }
    }

    /**
     * Get Informations on the FastCGI application
     *
     * @param array $requestedInfo information to retrieve
     * @return array
     */
    public function getValues(array $requestedInfo)
    {
        $this->connect();

        $request = '';
        foreach ($requestedInfo as $info) {
            $request .= $this->buildNvpair($info, '');
        }
        fwrite($this->_sock, $this->buildPacket(self::GET_VALUES, $request, 0));

        $resp = $this->readPacket();
        if ($resp['type'] == self::GET_VALUES_RESULT) {
            return $this->readNvpair($resp['content'], $resp['length']);
        } else {
            throw new \Exception('Unexpected response type, expecting GET_VALUES_RESULT');
        }
    }

    /**
     * Execute a request to the FastCGI application
     *
     * @param array $params Array of parameters
     * @param String $stdin Content
     * @return String
     */
    public function request(array $params, $stdin)
    {
        $id = $this->async_request($params, $stdin);
        return $this->wait_for_response($id);
    }

    /**
     * Execute a request to the FastCGI application asyncronously
     * 
     * This sends request to application and returns the assigned ID for that request.
     *
     * You should keep this id for later use with wait_for_response(). Ids are chosen randomly
     * rather than seqentially to guard against false-positives when using persistent sockets.
     * In that case it is possible that a delayed response to a request made by a previous script 
     * invocation comes back on this socket and is mistaken for response to request made with same ID
     * during this request.
     *
     * @param array $params Array of parameters
     * @param String $stdin Content
     * @return Integer
     */
    public function async_request(array $params, $stdin)
    {
        $this->connect();

        // Pick random number between 1 and max 16 bit unsigned int 65535
        $id = mt_rand(1, (1 << 16) - 1);

        // Using persistent sockets implies you want them keept alive by server!
        $keepAlive = intval($this->_keepAlive || $this->_persistentSocket);

        $request = $this->buildPacket(self::BEGIN_REQUEST
                                     ,chr(0) . chr(self::RESPONDER) . chr($keepAlive) . str_repeat(chr(0), 5)
                                     ,$id
                                     );

        $paramsRequest = '';
        foreach ($params as $key => $value) {
            $paramsRequest .= $this->buildNvpair($key, $value, $id);
        }
        if ($paramsRequest) {
            $request .= $this->buildPacket(self::PARAMS, $paramsRequest, $id);
        }
        $request .= $this->buildPacket(self::PARAMS, '', $id);

        if ($stdin) {
            $request .= $this->buildPacket(self::STDIN, $stdin, $id);
        }
        $request .= $this->buildPacket(self::STDIN, '', $id);

        if (fwrite($this->_sock, $request) === false || fflush($this->_sock) === false) {

            $info = stream_get_meta_data($this->_sock);

            if ($info['timed_out']) {
                throw new TimedOutException('Write timed out');
            }

            // Broken pipe, tear down so future requests might succeed
            fclose($this->_sock);
            throw new \Exception('Failed to write request to socket');
        }

        $this->_requests[$id] = array(
            'state' => self::REQ_STATE_WRITTEN,
            'response' => null
        );

        return $id;
    }

    /**
     * Blocking call that waits for response to specific request
     * 
     * @param Integer $requestId
     * @param Integer $timeoutMs [optional] the number of milliseconds to wait. Defaults to the ReadWriteTimeout value set.
     * @return string  response body
     */
    public function wait_for_response($requestId, $timeoutMs = 0) {

        if (!isset($this->_requests[$requestId])) {
            throw new \Exception('Invalid request id given');
        }

        // If we already read the response during an earlier call for different id, just return it
        if ($this->_requests[$requestId]['state'] == self::REQ_STATE_OK
            || $this->_requests[$requestId]['state'] == self::REQ_STATE_ERR
            ) {
            return $this->_requests[$requestId]['response'];
        }

        if ($timeoutMs > 0) {
            // Reset timeout on socket for now
            $this->set_ms_timeout($timeoutMs);
        } else {
            $timeoutMs = $this->_readWriteTimeout;
        }

        // Need to manually check since we might do several reads none of which timeout themselves
        // but still not get the response requested
        $startTime = microtime(true);

        do {
            $resp = $this->readPacket();

            if ($resp['type'] == self::STDOUT || $resp['type'] == self::STDERR) {
                if ($resp['type'] == self::STDERR) {
                    $this->_requests[$resp['requestId']]['state'] = self::REQ_STATE_ERR;
                }
                $this->_requests[$resp['requestId']]['response'] .= $resp['content'];
            }
            if ($resp['type'] == self::END_REQUEST) {
                $this->_requests[$resp['requestId']]['state'] = self::REQ_STATE_OK;
                if ($resp['requestId'] == $requestId) { 
                    break;
                }
            }
            if (microtime(true) - $startTime >= ($timeoutMs * 1000)) {
                // Reset
                $this->set_ms_timeout($this->_readWriteTimeout);
                throw new \Exception('Timed out');
            }
        } while ($resp);

        if (!is_array($resp)) {
            $info = stream_get_meta_data($this->_sock);

            // We must reset timeout but it must be AFTER we get info
            $this->set_ms_timeout($this->_readWriteTimeout);

            if ($info['timed_out']) {
                throw new TimedOutException('Read timed out');
            }

            if ($info['unread_bytes'] == 0
                    && $info['blocked']
                    && $info['eof']) {
                throw new ForbiddenException('Not in white list. Check listen.allowed_clients.');
            }

            throw new \Exception('Read failed');
        }

        // Reset timeout
        $this->set_ms_timeout($this->_readWriteTimeout);

        switch (ord($resp['content']{4})) {
            case self::CANT_MPX_CONN:
                throw new \Exception('This app can\'t multiplex [CANT_MPX_CONN]');
                break;
            case self::OVERLOADED:
                throw new \Exception('New request rejected; too busy [OVERLOADED]');
                break;
            case self::UNKNOWN_ROLE:
                throw new \Exception('Role value not known [UNKNOWN_ROLE]');
                break;
            case self::REQUEST_COMPLETE:
                return $this->_requests[$requestId]['response'];
        }
    }
}
