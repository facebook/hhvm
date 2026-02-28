Copyright (c) 2019, 2020, Oracle and/or its affiliates. All rights reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2.0,
as published by the Free Software Foundation.

This program is also distributed with certain software (including
but not limited to OpenSSL) that is licensed under separate terms,
as designated in a particular file or component or in included license
documentation.  The authors of MySQL hereby grant you an additional
permission to link the program and your derivative works with the
separately licensed software that they have included with MySQL.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License, version 2.0, for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

FREQUENTLY ASKED QUESTIONS (FAQ)
about error messages

1  What kinds of messages exist?

I  Messages that are built into certain components
   and that are not loaded by the server at runtime.
   They can not be localized; in the canonical version
   of the MySQL software, they are in English.

a) There are errors thrown by the operating system
   (cf. errno / strerror()).

b) The client library (libmysql / C-API) can throw
   certain errors on the client side. If for instance
   the client cannot connect to the server, we cannot
   receive an error message saying so from the server,
   so these messages are built into the client library.

c) "Global errors" are error codes / messages used by
   the mysys library. This library is used by the
   MySQL server as well as some of its clients.
   These messages are built into the mysys library.

II Messages that are loaded at runtime by the MySQL server.
   These messages can be localized; in the canonical version
   of the software, the default language is english. If a
   given message has not been translated to the configured
   language, the server will fall back to the english version
   of this message.
   The messages and their translations are configured in
   text files in the source tree. During the build process,
   files usable by the server (once for each language
   translations exist for in the input files) will be
   created.

d) Messages sent by the server to its clients.
   They are located in the file, messages_to_clients.txt
   in the same directory as these instructions.
   This is a different set of messages from that mentioned
   in b).
   See below for more information.

e) Messages sent by the server to the error log.
   They are located in the file, messages_to_error_log.txt
   in the same directory as these instructions.
   See below for more information.

These instructions are concerned with the messages
of type II.

These messages have a symbol ("ER_I_AM_A_TEAPOT"),
a number ("MY-000418"), and a message ("I am a teapot!").
The message may exist in various translations, and may
be changed (for clarity etc.) over time.

-> Message text may change, message signatures
   (the number and type of % subsitutions)
   currently must not.

-> Error numbers ("MY-000418") may not change,
   except for messages that exist only in trunk.

-> Error symbols may change, but this is strongly
   discouraged as user configurations (error log
   filtering etc.) may depend on these names.


2  What are ranges?

   In the simplest case, defining 5 error messages
   will give them the error numbers 1-5.
   This means error numbers will be assigned in the
   chronological order messages are added to the source.

   However, messages can be grouped in non-consecutive
   error number ranges. If each range is larger than the
   expect number of messages, there'll be some free "slots"
   (i.e. error numbers) at the end of each range in case
   we need them later. This lets us group messages/error
   number, which serves several purposes:

   -> We can keep all messages the server sends to clients
      in one range (error numbers less than 10,000), and
      all messages it writes to the error log in another
      range (error numbers 10,000 and greater).
      This way, the server can easily identify which
      group a message belongs to, and throw an error
      if the wrong type is sent to a given destination.

   -> We can keep all messages for a specific subsystem
      (e.g. X) together in its own range. Without ranges,
      messages for all subsystems would be numbered in
      the order they are added to the MySQL server's source,
      rather than by subsystem.

   -> Having different ranges for each version of the
      MySQL server lets us add messages to older versions
      of the server as required by bugfixing, and upmerge
      them safely:

      Suppose 5.6 has messages 3 messages, and 5.7 has
      the same 3 messages, but additionally 2 new ones
      for the features added in 5.7.

      Suppose also that we need to fix an issue in 5.6
      and above that requires a new message.

      - Without ranges, we'd start with the error numbers
        { 1, 2, 3 } in 5.6, and { 1, 2, 3, 4, 5 } in 5.7.
        Adding the new message to 5.6 would be simple;
        we'd append it with error number 4.

        5.7 however already has its own, different error
        number 4; appending the new message at the end
        of the 5.7 messages would give it different
        error numbers in 5.6 and 5.7, which would make
        the documentation confusing.

        Inserting the new message in 5.7's error list
        so it would have the same number as in 5.6
        would mean changing the error number of the
        message that previously was error number 4 in 5.7,
        and documentation would yet again be confusing.

      - With ranges, suppose we reserve the error numbers
        [1;10[ for 5.6, [10;20[ for 5.7, and so on.
        Now, we can safely append the new message as
        error number 4 to 5.6, and then upmerge it into
        5.7. The messages of 5.7 start at error number
        10, so upmerging message number 4 from 5.6 is safe,
        as number is not 4 used in 5.7; it's reserved for
        5.6 messages.



3  Where do I add server-to-client messages?
   (Messages used with my_error(), push_warning_printf())

   Messages that the server sends to its clients
   go into the file messages_to_clients.txt
   in the same directory as these instructions.
   Open that file, find the end of error number
   range for the server version you are adding
   the message to, and append your message.


4  Where do I add messages intended for the error log?
   (Messages used with LogErr() / LogEvent())

   Messages intended for the error log (as opposed to
   server-to-client messages) go into the file
   messages_to_error_log.txt in the same directory
   as these instructions. Open that file, find the
   end of error number range for the server version
   you are adding the message to, and append your message.


5  What do I do if a message needs to go to both the
   error log AND the client?

   If your message needs to be sent both to the client and to the
   error log, please add it twice, once in each of the above files.

   Several ranges may exist for each, pick the one corresponding
   to the lowest MySQL server version for which you are adding
   the message.

   Then, place two calls in the code (e.g. my_error() to send the
   one message to the client, and e.g. LogErr() to send the other
   message to the error log).

   If in doubt, use ER_ as the prefix for the error-log message,
   and ER_DA_ ("diagnostics area") for the one intended for the
   client, e.g.

   In messages_to_clients.txt:

     ER_DA_MY_MESSAGE
       eng "Hi!"

   In messages_to_error_log.txt:

     ER_MY_MESSAGE
       eng "Hi!"

   While both messages are allowed to have the same text, they
   do not need to do so, and you are encouraged to tailor each
   of the two strings to suit the intended audience and context.


6  I'm fixing existing code, how do I know where a message is sent?

   As a general guideline, if LogErr() or LogEvent() is used, the
   message is sent to the error log; if my_error() or
   push_warning_printf() is used, the message is sent to a client.

   -> executables other than the MySQL server may use their own
      wrappers around the relevant functions; see the documentation
      of that software for more information.

   -> within the MySQL server, the InnoDB storage engine uses
      its own wrapper code for the relevant functions; see the
      documentation for the InnoDB source code.

   -> within the MySQL server, the replication code (when used
      as a "client") may do transformations of error information;
      see the documentation of replication code for more information.


7  The test suite aborted with an assert() having to do with error numbers?

   This usually means that you tried to send a message intended for
   the error log to a client, or tried to write a message intended
   for the client to the error log. Find out whether two versions
   of this message already exist; if so, use the correct one;
   otherwise, add the missing message (see above for more information
   on how to do that).


8  Upmerging a new message into trunk failed, there already is a
   message in that place. What do I do?

   First, make sure you understand the general principle of
   per-version ranges described in answer 2.

   This is the exception from the rule described in answer 2,
   as trunk does not have its own range separate from that of
   the highest versioned tree.

   Therefore, error numbers in trunk, and only in trunk, may
   change during upmerge:

   For each range of messages in trunk, the order should become

     I    messages that existed in both in trunk and the lower branch
     II   messages you newly added to the lower branch
     III  messages that exist only in trunk

   This way, the message you add will have the same number in
   trunk as in the lower branch.

   However, it also means that the error numbers of new features
   in trunk, and ONLY in trunk, may change (as bug fixes are
   upmerged from the lower tree, and their messages inserted before
   the trunk-only ones, see also answer 2).

   It is more important to keep the error numbers that are in both
   trees (and therefore in released versions) stable between MySQL
   server versions than it is to keep the error numbers of new
   messages that exist only in trunk stable between commits to trunk.

   -> When error-numbers change, certain .result files may have to
      be re-recorded!

   This should ONLY happen when merging to trunk; once a branch is
   added for a new server version (e.g. "mysql-9.0"), a new range
   for 9.0 error log messages should be added, as well as a new
   range for 9.0 messages-to-clients.

   New messages to are most likely to be added for features (and
   therefore, in trunk), but if one were to be added in an 8.0
   bug fix, it would go at the end of the 8.0 range (the 8.0 range
   for error log or client messages, depending on the bug);
   this would not interfere with 9.0 messages as they'd be in a
   new, separate range. Interference / renumbering / merge problems
   should only ever occur between trunk and the most modern tree
   (mysql-x.y) below it!


9  What revelant files exist in the source tree?
   What relevant files are generated?
 
   The messages-to-clients are located in messages_to_clients.txt.
   These messages are sent by the MySQL server to its clients.

   The messages for the error log are located in
   messages_to_error_log.txt. These messages are
   written to the error log by the MySQL server.

   For each language in the files messages_to_clients.txt
   and messages_to_error_log.txt, a file errmsg.sys is created
   in a directory named for that language (e.g.
   share/english/errmsg.sys) at build time.

   At runtime, the server will read the messages from the errmsg.sys
   file containing the messages for the configured language.

   Additionally, the messages for the default language, English,
   are built into the server as a fallback so the MySQL server
   can still start (and report on the issue!) when it cannot find
   its errmsg.sys. To this end, mysqld_ername.h contains those
   messages as C code, while mysqld_error.h maps each symbol
   ("ER_MY_EXAMPLE_MESSAGE") to its error-number.


   The "global errors" defined by the mysys library are similarly
   built in from mysys/errors.cc (messages) and include/mysys_err.h
   (symbol to error number mapping), respectively.


   The client library (libmysql / C-API) also contains some error messages.
   These messages become part of theclient and let us provide diagnostics
   even when the server is not connected, or the server has no information
   about a client-side issue.

   -> These are not the same messages as the "messages intended
      for sending to clients"; this is further described in
      section 1-b above.

   -> As a simple guideline, client-library symbols begin with CR_,
      while server messages (both to the error log and for sending
      to clients) begin with ER_.

   For the client-library, mysqlclient_ername.h contains the union
   of messages, error numbers, and symbol names as C code. The file
   is generated from the symbol to error number mapping in errmsg.h
   and the  messages in errmsg.cc.
   The code in errmsg.cc serves as the client-library equivalent of
   messages_to_clients.txt.


10 What is the function of the OBSOLETE_ symbols?

   Symbols for obsolete messages are not defined in the build and
   therefore can't be used accidentally. The corresponding message
   however is still included in the errmsg.sys files (because older
   versions of the server may still use them), and the error number
   is still reserved (so the messages following below don't all have
   their error-number decreased by one when we make a message obsolete).
