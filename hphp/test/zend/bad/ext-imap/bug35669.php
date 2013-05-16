<?php
	$envelope["from"] = 'Santa <somewhere@northpole.gov>';
        $envelope["to"]  = 'The bad smurf <bad@smurf.com>';
        $envelope['date'] = 'Wed, 04 Jan 2006 19:24:43 -0500';
                
        $multipart["type"] = TYPEMULTIPART;
        $multipart["subtype"] = "MIXED";
        $body[] = $multipart; //add multipart stuff
        
        $textpart["type"] = TYPEMULTIPART;
        $textpart["subtype"] = "ALTERNATIVE";
        $body[] = $textpart; //add body part
        
        $plain["type"] = TYPETEXT;
        $plain["subtype"] = "PLAIN";
        $plain["charset"] = "iso-8859-1";
        $plain["encoding"] = ENCQUOTEDPRINTABLE;
        $plain["description"] = "Plaintype part of message";
        $plain['disposition'] = "inline";
        $plain["contents.data"] = 'See mom, it will crash';
        
        $body[] = $plain; //next add plain text part
        
        $html["type"] = TYPETEXT;
        $html["subtype"] = "HTML";
        $html["charset"] = "iso-8859-1";
        $html["encoding"] = ENCQUOTEDPRINTABLE;
        $html["description"] = "HTML part of message";
        $html['disposition'] = "inline";
        $html["contents.data"] = 'See mom, it will <b>crash</b>';
        
        $body[] = $html;
        
        echo imap_mail_compose($envelope, $body);
?>