/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/test/test_ext_mailparse.h"
#include "hphp/runtime/ext/ext.h"

///////////////////////////////////////////////////////////////////////////////

bool TestExtMailparse::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_mail);
  RUN_TEST(test_ezmlm_hash);
  RUN_TEST(test_mailparse_msg_create);
  RUN_TEST(test_mailparse_msg_free);
  RUN_TEST(test_mailparse_msg_parse_file);
  RUN_TEST(test_mailparse_msg_parse);
  RUN_TEST(test_mailparse_msg_extract_part_file);
  RUN_TEST(test_mailparse_msg_extract_whole_part_file);
  RUN_TEST(test_mailparse_msg_extract_part);
  RUN_TEST(test_mailparse_msg_get_part_data);
  RUN_TEST(test_mailparse_msg_get_part);
  RUN_TEST(test_mailparse_msg_get_structure);
  RUN_TEST(test_mailparse_rfc822_parse_addresses);
  RUN_TEST(test_mailparse_stream_encode);
  RUN_TEST(test_mailparse_uudecode_all);
  RUN_TEST(test_mailparse_determine_best_xfer_encoding);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtMailparse::test_mail() {
  //VCB("<?php ");
  return Count(true);
}

bool TestExtMailparse::test_ezmlm_hash() {
  VS(f_ezmlm_hash("foo"), 40);
  return Count(true);
}

bool TestExtMailparse::test_mailparse_msg_create() {
  const char *files[] = { "mime", "phpcvs1", "qp", "uue" };

  for (unsigned int i = 0; i < sizeof(files)/sizeof(files[0]); i++) {
    string file = files[i];
    string testname = "test/test_ext_mailparse." + file + ".txt";
    string expname = "test/test_ext_mailparse." + file + ".exp";

    Variant mime = f_mailparse_msg_create();
    PlainFile input;
    input.open(testname, "r");
    while (!input.eof()) {
      String data = input.read(1024);
      if (!data.isNull()) {
        f_mailparse_msg_parse(mime, data);
      }
    }
    input.close();

    Array arr = f_mailparse_msg_get_structure(mime);
    f_ob_start();
    echo("Message: "); echo(file.c_str()); echo("\n");
    for (ArrayIter iter(arr); iter; ++iter) {
      Variant partname = iter.second();
      int depth = f_count(f_explode(".", partname)) - 1;
      String indent = f_str_repeat("  ", depth * 2);

      Variant subpart = f_mailparse_msg_get_part(mime, partname);
      if (subpart.isNull()) {
        f_var_dump(partname); echo("\n");
        f_var_dump(arr);
        break;
      }

      Variant data = f_mailparse_msg_get_part_data(subpart);
      echo("\n"); echo(indent); echo("Part "); echo(partname); echo("\n");
      f_ksort(ref(data));
      for (ArrayIter iter(data); iter; ++iter) {
        String key = iter.first().toString();
        if (key != "headers" && key != "ending-pos-body") {
          echo(indent); echo(key); echo(" => ");
          f_var_dump(iter.second());
        }
      }
    }
    String output = f_ob_get_contents();

    Variant expect = f_file_get_contents(expname.c_str());
    VS(output, expect);
    f_ob_end_clean();
  }

  return Count(true);
}

bool TestExtMailparse::test_mailparse_msg_free() {
  f_mailparse_msg_free(uninit_null());
  return Count(true);
}

bool TestExtMailparse::test_mailparse_msg_parse_file() {
  return Count(true);
}

bool TestExtMailparse::test_mailparse_msg_parse() {
  static const StaticString
    s_headers("headers"),
    s_to("to");
  String text =
    "To: fred@bloggs.com\n"
    "To: wez@thebrainroom.com\n"
    "\n"
    "hello, this is some text=hello.";

  Variant mime = f_mailparse_msg_create();
  f_mailparse_msg_parse(mime, text);
  Variant data = f_mailparse_msg_get_part_data(mime);
  VS(data[s_headers][s_to], "fred@bloggs.com, wez@thebrainroom.com");
  return Count(true);
}

bool TestExtMailparse::test_mailparse_msg_extract_part_file() {
  // tested in test_mailparse_msg_extract_part()
  return Count(true);
}

bool TestExtMailparse::test_mailparse_msg_extract_whole_part_file() {
  // tested in test_mailparse_msg_extract_part()
  return Count(true);
}

bool TestExtMailparse::test_mailparse_msg_extract_part() {
  String text =
    "To: fred@bloggs.com\n"
    "Mime-Version: 1.0\n"
    "Content-Type: text/plain\n"
    "Subject: A simple MIME message\n"
    "\n"
    "hello, this is some text hello.\n"
    "blah blah blah.\n";

  Variant fp = f_tmpfile();
  f_fwrite(fp, text);
  f_rewind(fp);

  f_ob_start();

  Variant mime = f_mailparse_msg_create();
  f_mailparse_msg_parse(mime, text);

  echo("Extract to output\n");
  f_mailparse_msg_extract_part_file(mime, fp);

  echo("Extract and return as string\n");
  Variant result = f_mailparse_msg_extract_part_file(mime, fp, uninit_null());
  echo("-->\n");
  echo(result);

  echo("\nExtract to open file\n");
  Variant fpdest = f_tmpfile();
  f_mailparse_msg_extract_part_file(mime, fp, fpdest);
  echo("\nrewinding\n");
  f_rewind(fpdest);
  f_fpassthru(fpdest);

  echo("\nExtract via user function\n");
  f_mailparse_msg_extract_part_file(mime, fp);

  echo("\nExtract whole part to output\n");
  f_mailparse_msg_extract_whole_part_file(mime, fp);

  echo("\nExtract part from string to output\n");
  f_mailparse_msg_extract_part(mime, text);
  f_fclose(fpdest);
  f_fclose(fp);

  String output = f_ob_get_contents();
  f_ob_end_clean();

  VS(output,
     "Extract to output\n"
     "hello, this is some text hello.\n"
     "blah blah blah.\n"
     "Extract and return as string\n"
     "-->\n"
     "hello, this is some text hello.\n"
     "blah blah blah.\n"
     "\n"
     "Extract to open file\n"
     "\n"
     "rewinding\n"
     "hello, this is some text hello.\n"
     "blah blah blah.\n"
     "\n"
     "Extract via user function\n"
     "hello, this is some text hello.\n"
     "blah blah blah.\n"
     "\n"
     "Extract whole part to output\n"
     "To: fred@bloggs.com\n"
     "Mime-Version: 1.0\n"
     "Content-Type: text/plain\n"
     "Subject: A simple MIME message\n"
     "\n"
     "hello, this is some text hello.\n"
     "blah blah blah.\n"
     "\n"
     "Extract part from string to output\n"
     "hello, this is some text hello.\n"
     "blah blah blah.\n");

  return Count(true);
}

bool TestExtMailparse::test_mailparse_msg_get_part_data() {
  // tested in test_mailparse_msg_parse()
  return Count(true);
}

bool TestExtMailparse::test_mailparse_msg_get_part() {
  String msg =
    "Received: from mail pickup service by hotmail.com with Microsoft\n"
    "SMTPSVC;\n"
    "Sat, 18 Feb 2006 22:58:14 -0800\n"
    "Received: from 66.178.40.49 by BAY116-DAV8.phx.gbl with DAV;\n"
    "Sun, 19 Feb 2006 06:58:13 +0000\n"
    "\n"
    "test";

  Variant mail = f_mailparse_msg_create();
  f_mailparse_msg_parse(mail, msg);
  Array arr = f_mailparse_msg_get_structure(mail);
  for (ArrayIter iter(arr); iter; ++iter) {
    Variant section = f_mailparse_msg_get_part(mail, iter.second());
    Variant info = f_mailparse_msg_get_part_data(section);
    Array received =
      CREATE_VECTOR2("from mail pickup service by hotmail.com with Microsoft",
                     "from 66.178.40.49 by BAY116-DAV8.phx.gbl with DAV;");
    VS(info,
       Array(ArrayInit(11).
             set("headers", CREATE_MAP1("received", received)).
             set("starting-pos", 0).
             set("starting-pos-body", 200).
             set("ending-pos", 200).
             set("ending-pos-body", 200).
             set("line-count", 6).
             set("body-line-count", 0).
             set("charset", "us-ascii").
             set("transfer-encoding", "8bit").
             set("content-type", "text/plain").
             set("content-base", "/").
             create()));
  }

  return Count(true);
}

bool TestExtMailparse::test_mailparse_msg_get_structure() {
  // tested in test_mailparse_msg_create()
  return Count(true);
}

bool TestExtMailparse::test_mailparse_rfc822_parse_addresses() {
  static const StaticString
    s_display("display"),
    s_address("address"),
    s_is_group("is_group");
  Array addresses =
    CREATE_VECTOR2("\":sysmail\"@ Some-Group. Some-Org, Muhammed."
                   "(I am the greatest) Ali @(the)Vegas.WBA",
                   "\"strange\":\":sysmail\"@ Some-Group. Some-Org, Muhammed."
                   "(I am the greatest) Ali @(the)Vegas.WBA;");

  f_ob_start();

  for (ArrayIter iter(addresses); iter; ++iter) {
    Variant parsed = f_mailparse_rfc822_parse_addresses(iter.second());
    for (ArrayIter iter2(parsed); iter2; ++iter2) {
      Variant pair = iter2.second();
      echo(pair[s_display]); echo("\n");
      echo(pair[s_address]); echo("\n");
      if (pair[s_is_group].toBoolean()) {
        Variant sub = f_mailparse_rfc822_parse_addresses
          (f_substr(pair[s_address], 1, f_strlen(pair[s_address]) - 2));
        for (ArrayIter iter3(sub); iter3; ++iter3) {
          echo("   "); echo(iter3.second()[s_address]); echo("\n");
        }
      }
    }
    echo("...\n");
  }

  String output = f_ob_get_contents();
  f_ob_end_clean();
  VS(output,
     ":sysmail@Some-Group.Some-Org\n"
     "\":sysmail\"@Some-Group.Some-Org\n"
     "I am the greatest the\n"
     "Muhammed.Ali@Vegas.WBA\n"
     "...\n"
     "strange\n"
     ":\":sysmail\"@Some-Group.Some-Org,Muhammed.Ali@Vegas.WBA;\n"
     "   \":sysmail\"@Some-Group.Some-Org\n"
     "   Muhammed.Ali@Vegas.WBA\n"
     "...\n");

  return Count(true);
}

bool TestExtMailparse::test_mailparse_stream_encode() {
  {
    String text = "hello, this is some text=hello.";
    Variant fp = f_tmpfile();
    f_fwrite(fp, text); f_rewind(fp);
    Variant dest = f_tmpfile();
    f_mailparse_stream_encode(fp, dest, "quoted-printable");
    f_rewind(dest);
    Variant data = f_fread(dest, 2048);
    VS(data, "hello, this is some text=3Dhello.");
  }
  {
    String text =
      "To: fred@bloggs.com\n"
      "\n"
      "blah blah blah From blah $ \" & £ blah blah blah blah blah\n"
      "From the first of the month, things will be different!\n"
      "blah blah blah From blah\n"
      "Frome is a town in Somerset.";

    f_ob_start();

    Variant fp = f_tmpfile();
    f_fwrite(fp, text);
    f_rewind(fp);

    Variant fpdest = f_tmpfile();
    f_mailparse_stream_encode(fp, fpdest, "quoted-printable");
    f_rewind(fpdest);
    f_fpassthru(fpdest);

    f_fclose(fp);
    f_fclose(fpdest);

    String output = f_ob_get_contents();
    f_ob_end_clean();
    VS(output,
       "To: fred@bloggs.com\r\n"
       "\r\n"
       "blah blah blah From blah $ \" & =C2=A3 blah blah blah blah blah\r\n"
       "=46rom the first of the month, things will be different!\r\n"
       "blah blah blah From blah\r\n"
       "Frome is a town in Somerset.");
  }

  return Count(true);
}

bool TestExtMailparse::test_mailparse_uudecode_all() {
  static const StaticString s_filename("filename");
  String text =
    "To: fred@bloggs.com\n"
    "\n"
    "hello, this is some text hello.\n"
    "blah blah blah.\n"
    "\n"
    "begin 644 test.txt\n"
    "/=&AI<R!I<R!A('1E<W0*\n"
    "`\n"
    "end";

  f_ob_start();

  Variant fp = f_tmpfile();
  f_fwrite(fp, text);

  Variant data = f_mailparse_uudecode_all(fp);
  echo("BODY\n");
  f_readfile(data[0][s_filename]);
  echo("UUE\n");
  f_readfile(data[1][s_filename]);

  f_unlink(data[0][s_filename]);
  f_unlink(data[1][s_filename]);

  String output = f_ob_get_contents();
  f_ob_end_clean();
  VS(output,
     "BODY\n"
     "To: fred@bloggs.com\n"
     "\n"
     "hello, this is some text hello.\n"
     "blah blah blah.\n"
     "\n"
     "UUE\n"
     "this is a test\n");

  return Count(true);
}

bool TestExtMailparse::test_mailparse_determine_best_xfer_encoding() {
  return Count(true);
}
