<?php
$headers = <<<HERE
Return-Path: <internals-return-5651-***=***.example.com@lists.php.net>
Received: from pb1.pair.com (pb1.pair.com [16.92.131.4]) by ***.example.com 
    (8.12.10/8.12.10/1970-09-30) with SMTP id hALLmpea023899 for
    <***@***.example.com>; Sat, 22 Jan 1970 06:48:51 +0900 (JST)
    (envelope-from
    internals-return-5651-***=***.example.com@lists.php.net)
Received: (qmail 63472 invoked by uid 1010); 1 Jan 1970 0:00:00 -0000
Mailing-List: contact internals-help@lists.php.net; run by ezmlm
Precedence: bulk
List-Help: <mailto:internals-help@lists.php.net>
List-Unsubscribe: <mailto:internals-unsubscribe@lists.php.net>
List-Post: <mailto:internals@lists.php.net>
Delivered-To: mailing list internals@lists.php.net
Received: (qmail 63459 invoked by uid 1010); 1 Jan 1970 0:00:00 -0000
Delivered-To: ezmlm-scan-internals@lists.php.net
Delivered-To: ezmlm-internals@lists.php.net
Date: Thu, 1 Jan 1970 00:00:00 -0000 (GMT)
From: *** *** *** <***@***.example.com>
X-X-Sender: ***@***.example.com 
To: internals@lists.php.net
Message-Id: <Pine.LNX.4.58.************@***.example.com>
MIME-Version: 1.0
Content-Type: TEXT/PLAIN; charset=US-ASCII
Subject: [PHP-DEV] [ICONV] test for =?US-ASCII?Q?iconv_mime_decode_headers=28=29?=
X-UIDL: @eH!!h2:!!EOS!!A_c"!
HERE;
var_dump(iconv_mime_decode_headers($headers));
?>