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

#include "hphp/test/test_ext_imagesprite.h"
#include "hphp/runtime/ext/ext_imagesprite.h"
#include "hphp/runtime/ext/ext_url.h"

IMPLEMENT_SEP_EXTENSION_TEST(Imagesprite);
///////////////////////////////////////////////////////////////////////////////

bool TestExtImagesprite::RunTests(const std::string &which) {
  bool ret = true;

  DECLARE_TEST_FUNCTIONS("");

  RUN_TEST(test_ImageSprite);
  RUN_TEST(test_addFile);
  RUN_TEST(test_addString);
  RUN_TEST(test_addUrl);
  RUN_TEST(test_clear);
  RUN_TEST(test_loadDims);
  RUN_TEST(test_loadImages);
  RUN_TEST(test_map);

  sprite.reset();

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtImagesprite::preTest() {
  sprite = p_ImageSprite(NEWOBJ(c_ImageSprite)());

  return true;
}

bool TestExtImagesprite::test_ImageSprite() {
  VS(sprite->m_img_errors.size(), 0);
  VS(sprite->m_sprite_errors.size(), 0);
  VS((int) sprite->m_image_data.size(), 0);

  return Count(true);
}

bool TestExtImagesprite::test_addFile() {
  sprite->t_addfile("test/images/php.gif");
  VS((int) sprite->m_image_data.size(), 1);
  VS(sprite->m_image_data["test/images/php.gif"] == nullptr, false);
  VS(sprite->m_img_errors.size(), 0);

  sprite->t_addfile("test/images/php.gif");
  VS((int) sprite->m_image_data.size(), 1);
  VS(sprite->m_image_data["test/images/php.gif"] == nullptr, false);
  VS(sprite->m_img_errors.size(), 0);

  sprite->t_addfile("test/images/246x247.png");
  VS((int) sprite->m_image_data.size(), 2);
  VS(sprite->m_image_data["test/images/php.gif"] == nullptr, false);
  VS(sprite->m_img_errors.size(), 0);

  sprite->t_loadimages(true);
  VS(sprite->m_img_errors.size(), 0);

  sprite->t_addfile("invalid-image");
  VS((int) sprite->m_image_data.size(), 3);
  VS(sprite->m_image_data["invalid-image"] == nullptr, false);
  VS(sprite->m_img_errors.size(), 0);

  sprite->t_loadimages(true);
  VS(sprite->m_img_errors.size(), 1);

  return Count(true);
}
bool TestExtImagesprite::test_addString() {
  String img = "R0lGODlhAQABAIAAAP///wAAACH5BAEAAAAALAAAAAABAAEAAAICRAEAOw==";
  sprite->t_addstring("transparent", f_base64_decode(img));
  VS((int) sprite->m_image_data.size(), 1);
  VS(sprite->m_image_data["transparent"] == nullptr, false);
  VS(sprite->m_img_errors.size(), 0);

  sprite->t_addstring("transparent", f_base64_decode(img));
  VS((int) sprite->m_image_data.size(), 1);
  VS(sprite->m_image_data["transparent"] == nullptr, false);
  VS(sprite->m_img_errors.size(), 0);

  sprite->t_addstring("image-2", f_base64_decode(img));
  VS((int) sprite->m_image_data.size(), 2);
  VS(sprite->m_image_data["image-2"] == nullptr, false);
  VS(sprite->m_img_errors.size(), 0);

  sprite->t_loadimages(true);
  VS(sprite->m_img_errors.size(), 0);

  sprite->t_addstring("invalid-image", "invalid-image");
  VS((int) sprite->m_image_data.size(), 3);
  VS(sprite->m_image_data["invalid-image"] == nullptr, false);
  VS(sprite->m_img_errors.size(), 0);

  sprite->t_loadimages(true);
  VS(sprite->m_img_errors.size(), 1);

  return Count(true);
}
bool TestExtImagesprite::test_addUrl() {
  sprite->t_addurl("http://www.facebook.com/images/icons/like.png");
  VS((int) sprite->m_image_data.size(), 1);
  VS(
    sprite->m_image_data["http://www.facebook.com/images/icons/like.png"]
      == nullptr,
    false);
  VS(sprite->m_img_errors.size(), 0);

  sprite->t_addurl("http://www.facebook.com/images/icons/like.png");
  VS((int) sprite->m_image_data.size(), 1);
  VS(
    sprite->m_image_data["http://www.facebook.com/images/icons/like.png"]
      == nullptr,
    false);
  VS(sprite->m_img_errors.size(), 0);

  sprite->t_addurl("http://www.facebook.com/images/icons/friend.gif");
  VS((int) sprite->m_image_data.size(), 2);
  VS(
    sprite->m_image_data["http://www.facebook.com/images/icons/friend.gif"]
      == nullptr,
    false);
  VS(sprite->m_img_errors.size(), 0);

  sprite->t_loadimages(true);
  VS(sprite->m_img_errors.size(), 0);

  sprite->t_addurl("http://www.facebook.com/images/invalid-image.txt");
  VS((int) sprite->m_image_data.size(), 3);
  VS(
    sprite->m_image_data["http://www.facebook.com/images/invalid-image.txt"]
      == nullptr,
    false);
  VS(sprite->m_img_errors.size(), 0);

  sprite->t_loadimages(true);
  VS(sprite->m_img_errors.size(), 1);

  return Count(true);
}
bool TestExtImagesprite::test_clear() {
  VS((int) sprite->m_image_data.size(), 0);
  sprite->t_addfile("test/images/php.gif");
  VS((int) sprite->m_image_data.size(), 1);
  sprite->t_clear("test/images/php.gif");
  VS((int) sprite->m_image_data.size(), 0);

  sprite->t_addfile("test/images/php.gif");
  sprite->t_addfile("test/images/246x247.png");
  VS((int) sprite->m_image_data.size(), 2);

  sprite->t_clear("test/images/php.gif");
  VS((int) sprite->m_image_data.size(), 1);

  sprite->t_addfile("test/images/php.gif");
  sprite->t_clear();
  VS((int) sprite->m_image_data.size(), 0);

  return Count(true);
}

static const StaticString
  s_width("width"),
  s_height("height");

bool TestExtImagesprite::test_loadDims() {
  sprite->t_addfile("test/images/php.gif");
  sprite->t_loaddims(true);
  VS(sprite->m_image_data["test/images/php.gif"]->m_width, 120);
  VS(sprite->m_image_data["test/images/php.gif"]->m_height, 67);
  VS(sprite->m_image_data["test/images/php.gif"]->m_image == nullptr, false);

  sprite->t_clear();
  Array dims = Array::Create();
  dims.set(s_width, 1);
  dims.set(s_height, 1);
  sprite->t_addfile("test/images/php.gif", dims);
  sprite->t_loaddims(true);
  VS(sprite->m_image_data["test/images/php.gif"]->m_width, 1);
  VS(sprite->m_image_data["test/images/php.gif"]->m_height, 1);
  VS(sprite->m_image_data["test/images/php.gif"]->m_image == nullptr, true);

  sprite->t_clear();
  sprite->t_addurl("http://www.facebook.com/images/icons/like.png", -1, dims);
  sprite->t_loaddims(true);
  VS(
    sprite->m_image_data["http://www.facebook.com/images/icons/like.png"]
      ->m_width,
    1);
  VS(
    sprite->m_image_data["http://www.facebook.com/images/icons/like.png"]
      ->m_height,
    1);
  VS(
    sprite->m_image_data["http://www.facebook.com/images/icons/like.png"]
      ->m_image == nullptr,
    true);

  return Count(true);
}
bool TestExtImagesprite::test_loadImages() {
  sprite->t_addfile("test/images/php.gif");
  sprite->t_loadimages(true);
  VS(sprite->m_image_data["test/images/php.gif"]->m_width, 120);
  VS(sprite->m_image_data["test/images/php.gif"]->m_height, 67);
  VS(sprite->m_image_data["test/images/php.gif"]->m_image == nullptr, false);

  sprite->t_clear();
  Array dims = Array::Create();
  dims.set(s_width, 1);
  dims.set(s_height, 1);
  sprite->t_addfile("test/images/php.gif", dims);
  sprite->t_loadimages(true);
  VS(sprite->m_image_data["test/images/php.gif"]->m_width, 120);
  VS(sprite->m_image_data["test/images/php.gif"]->m_height, 67);
  VS(sprite->m_image_data["test/images/php.gif"]->m_image == nullptr, false);

  // XXX disabled until we work out flaky network issues. t2183444
#if 0
  sprite->t_clear();
  sprite->t_addurl("http://www.facebook.com/images/icons/like.png", -1, dims);
  sprite->t_loadimages(true);
  VS(
    sprite->m_image_data["http://www.facebook.com/images/icons/like.png"]
      ->m_width,
    16);
  VS(
    sprite->m_image_data["http://www.facebook.com/images/icons/like.png"]
      ->m_height,
    16);
  VS(
    sprite->m_image_data["http://www.facebook.com/images/icons/like.png"]
      ->m_image
      == nullptr,
    false);
#endif
  return Count(true);
}
bool TestExtImagesprite::test_map() {
  static const StaticString
    s_images("images"),
    s_width("width"),
    s_height("height"),
    s_x("x"),
    s_y("y"),
    testpath("test/images/php.gif");

  sprite->t_addfile(testpath);
  VS(sprite->m_current, false);
  Array map = sprite->t_mapping();
  VS(map[s_images].toArray()[testpath].toArray()[s_width].toInt32(), 120);
  VS(map[s_images].toArray()[testpath].toArray()[s_height].toInt32(), 67);
  VS(map[s_images].toArray()[testpath].toArray()[s_x].toInt32(), 0);
  VS(map[s_images].toArray()[testpath].toArray()[s_y].toInt32(), 0);
  VS(map[s_width].toInt32(), 121);
  VS(map[s_height].toInt32(), 68);
  VS(sprite->m_current, true);

  sprite->t_clear();
  Array dims = Array::Create();
  dims.set(s_width, 1);
  dims.set(s_height, 1);
  sprite->t_addfile(testpath, dims);
  VS(sprite->m_current, false);
  map = sprite->t_mapping();
  VS(map[s_images].toArray()[testpath].toArray()[s_width].toInt32(), 1);
  VS(map[s_images].toArray()[testpath].toArray()[s_height].toInt32(), 1);
  VS(map[s_images].toArray()[testpath].toArray()[s_x].toInt32(), 0);
  VS(map[s_images].toArray()[testpath].toArray()[s_y].toInt32(), 0);
  VS(map[s_width].toInt32(), 2);
  VS(map[s_height].toInt32(), 2);
  VS(sprite->m_current, true);

  return Count(true);
}
