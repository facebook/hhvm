/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <test/test_ext_magick.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtMagick::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_magickgetcopyright);
  RUN_TEST(test_magickgethomeurl);
  RUN_TEST(test_magickgetpackagename);
  RUN_TEST(test_magickgetquantumdepth);
  RUN_TEST(test_magickgetreleasedate);
  RUN_TEST(test_magickgetresourcelimit);
  RUN_TEST(test_magickgetversion);
  RUN_TEST(test_magickgetversionnumber);
  RUN_TEST(test_magickgetversionstring);
  RUN_TEST(test_magickqueryconfigureoption);
  RUN_TEST(test_magickqueryconfigureoptions);
  RUN_TEST(test_magickqueryfonts);
  RUN_TEST(test_magickqueryformats);
  RUN_TEST(test_magicksetresourcelimit);
  RUN_TEST(test_newdrawingwand);
  RUN_TEST(test_newmagickwand);
  RUN_TEST(test_newpixeliterator);
  RUN_TEST(test_newpixelregioniterator);
  RUN_TEST(test_newpixelwand);
  RUN_TEST(test_newpixelwandarray);
  RUN_TEST(test_newpixelwands);
  RUN_TEST(test_destroydrawingwand);
  RUN_TEST(test_destroymagickwand);
  RUN_TEST(test_destroypixeliterator);
  RUN_TEST(test_destroypixelwand);
  RUN_TEST(test_destroypixelwandarray);
  RUN_TEST(test_destroypixelwands);
  RUN_TEST(test_isdrawingwand);
  RUN_TEST(test_ismagickwand);
  RUN_TEST(test_ispixeliterator);
  RUN_TEST(test_ispixelwand);
  RUN_TEST(test_cleardrawingwand);
  RUN_TEST(test_clearmagickwand);
  RUN_TEST(test_clearpixeliterator);
  RUN_TEST(test_clearpixelwand);
  RUN_TEST(test_clonedrawingwand);
  RUN_TEST(test_clonemagickwand);
  RUN_TEST(test_wandgetexception);
  RUN_TEST(test_wandgetexceptionstring);
  RUN_TEST(test_wandgetexceptiontype);
  RUN_TEST(test_wandhasexception);
  RUN_TEST(test_drawaffine);
  RUN_TEST(test_drawannotation);
  RUN_TEST(test_drawarc);
  RUN_TEST(test_drawbezier);
  RUN_TEST(test_drawcircle);
  RUN_TEST(test_drawcolor);
  RUN_TEST(test_drawcomment);
  RUN_TEST(test_drawcomposite);
  RUN_TEST(test_drawellipse);
  RUN_TEST(test_drawgetclippath);
  RUN_TEST(test_drawgetcliprule);
  RUN_TEST(test_drawgetclipunits);
  RUN_TEST(test_drawgetexception);
  RUN_TEST(test_drawgetexceptionstring);
  RUN_TEST(test_drawgetexceptiontype);
  RUN_TEST(test_drawgetfillalpha);
  RUN_TEST(test_drawgetfillcolor);
  RUN_TEST(test_drawgetfillopacity);
  RUN_TEST(test_drawgetfillrule);
  RUN_TEST(test_drawgetfont);
  RUN_TEST(test_drawgetfontfamily);
  RUN_TEST(test_drawgetfontsize);
  RUN_TEST(test_drawgetfontstretch);
  RUN_TEST(test_drawgetfontstyle);
  RUN_TEST(test_drawgetfontweight);
  RUN_TEST(test_drawgetgravity);
  RUN_TEST(test_drawgetstrokealpha);
  RUN_TEST(test_drawgetstrokeantialias);
  RUN_TEST(test_drawgetstrokecolor);
  RUN_TEST(test_drawgetstrokedasharray);
  RUN_TEST(test_drawgetstrokedashoffset);
  RUN_TEST(test_drawgetstrokelinecap);
  RUN_TEST(test_drawgetstrokelinejoin);
  RUN_TEST(test_drawgetstrokemiterlimit);
  RUN_TEST(test_drawgetstrokeopacity);
  RUN_TEST(test_drawgetstrokewidth);
  RUN_TEST(test_drawgettextalignment);
  RUN_TEST(test_drawgettextantialias);
  RUN_TEST(test_drawgettextdecoration);
  RUN_TEST(test_drawgettextencoding);
  RUN_TEST(test_drawgettextundercolor);
  RUN_TEST(test_drawgetvectorgraphics);
  RUN_TEST(test_drawline);
  RUN_TEST(test_drawmatte);
  RUN_TEST(test_drawpathclose);
  RUN_TEST(test_drawpathcurvetoabsolute);
  RUN_TEST(test_drawpathcurvetoquadraticbezierabsolute);
  RUN_TEST(test_drawpathcurvetoquadraticbezierrelative);
  RUN_TEST(test_drawpathcurvetoquadraticbeziersmoothabsolute);
  RUN_TEST(test_drawpathcurvetoquadraticbeziersmoothrelative);
  RUN_TEST(test_drawpathcurvetorelative);
  RUN_TEST(test_drawpathcurvetosmoothabsolute);
  RUN_TEST(test_drawpathcurvetosmoothrelative);
  RUN_TEST(test_drawpathellipticarcabsolute);
  RUN_TEST(test_drawpathellipticarcrelative);
  RUN_TEST(test_drawpathfinish);
  RUN_TEST(test_drawpathlinetoabsolute);
  RUN_TEST(test_drawpathlinetohorizontalabsolute);
  RUN_TEST(test_drawpathlinetohorizontalrelative);
  RUN_TEST(test_drawpathlinetorelative);
  RUN_TEST(test_drawpathlinetoverticalabsolute);
  RUN_TEST(test_drawpathlinetoverticalrelative);
  RUN_TEST(test_drawpathmovetoabsolute);
  RUN_TEST(test_drawpathmovetorelative);
  RUN_TEST(test_drawpathstart);
  RUN_TEST(test_drawpoint);
  RUN_TEST(test_drawpolygon);
  RUN_TEST(test_drawpolyline);
  RUN_TEST(test_drawrectangle);
  RUN_TEST(test_drawrender);
  RUN_TEST(test_drawrotate);
  RUN_TEST(test_drawroundrectangle);
  RUN_TEST(test_drawscale);
  RUN_TEST(test_drawsetclippath);
  RUN_TEST(test_drawsetcliprule);
  RUN_TEST(test_drawsetclipunits);
  RUN_TEST(test_drawsetfillalpha);
  RUN_TEST(test_drawsetfillcolor);
  RUN_TEST(test_drawsetfillopacity);
  RUN_TEST(test_drawsetfillpatternurl);
  RUN_TEST(test_drawsetfillrule);
  RUN_TEST(test_drawsetfont);
  RUN_TEST(test_drawsetfontfamily);
  RUN_TEST(test_drawsetfontsize);
  RUN_TEST(test_drawsetfontstretch);
  RUN_TEST(test_drawsetfontstyle);
  RUN_TEST(test_drawsetfontweight);
  RUN_TEST(test_drawsetgravity);
  RUN_TEST(test_drawsetstrokealpha);
  RUN_TEST(test_drawsetstrokeantialias);
  RUN_TEST(test_drawsetstrokecolor);
  RUN_TEST(test_drawsetstrokedasharray);
  RUN_TEST(test_drawsetstrokedashoffset);
  RUN_TEST(test_drawsetstrokelinecap);
  RUN_TEST(test_drawsetstrokelinejoin);
  RUN_TEST(test_drawsetstrokemiterlimit);
  RUN_TEST(test_drawsetstrokeopacity);
  RUN_TEST(test_drawsetstrokepatternurl);
  RUN_TEST(test_drawsetstrokewidth);
  RUN_TEST(test_drawsettextalignment);
  RUN_TEST(test_drawsettextantialias);
  RUN_TEST(test_drawsettextdecoration);
  RUN_TEST(test_drawsettextencoding);
  RUN_TEST(test_drawsettextundercolor);
  RUN_TEST(test_drawsetvectorgraphics);
  RUN_TEST(test_drawsetviewbox);
  RUN_TEST(test_drawskewx);
  RUN_TEST(test_drawskewy);
  RUN_TEST(test_drawtranslate);
  RUN_TEST(test_pushdrawingwand);
  RUN_TEST(test_drawpushclippath);
  RUN_TEST(test_drawpushdefs);
  RUN_TEST(test_drawpushpattern);
  RUN_TEST(test_popdrawingwand);
  RUN_TEST(test_drawpopclippath);
  RUN_TEST(test_drawpopdefs);
  RUN_TEST(test_drawpoppattern);
  RUN_TEST(test_magickadaptivethresholdimage);
  RUN_TEST(test_magickaddimage);
  RUN_TEST(test_magickaddnoiseimage);
  RUN_TEST(test_magickaffinetransformimage);
  RUN_TEST(test_magickannotateimage);
  RUN_TEST(test_magickappendimages);
  RUN_TEST(test_magickaverageimages);
  RUN_TEST(test_magickblackthresholdimage);
  RUN_TEST(test_magickblurimage);
  RUN_TEST(test_magickborderimage);
  RUN_TEST(test_magickcharcoalimage);
  RUN_TEST(test_magickchopimage);
  RUN_TEST(test_magickclipimage);
  RUN_TEST(test_magickclippathimage);
  RUN_TEST(test_magickcoalesceimages);
  RUN_TEST(test_magickcolorfloodfillimage);
  RUN_TEST(test_magickcolorizeimage);
  RUN_TEST(test_magickcombineimages);
  RUN_TEST(test_magickcommentimage);
  RUN_TEST(test_magickcompareimages);
  RUN_TEST(test_magickcompositeimage);
  RUN_TEST(test_magickconstituteimage);
  RUN_TEST(test_magickcontrastimage);
  RUN_TEST(test_magickconvolveimage);
  RUN_TEST(test_magickcropimage);
  RUN_TEST(test_magickcyclecolormapimage);
  RUN_TEST(test_magickdeconstructimages);
  RUN_TEST(test_magickdescribeimage);
  RUN_TEST(test_magickdespeckleimage);
  RUN_TEST(test_magickdrawimage);
  RUN_TEST(test_magickechoimageblob);
  RUN_TEST(test_magickechoimagesblob);
  RUN_TEST(test_magickedgeimage);
  RUN_TEST(test_magickembossimage);
  RUN_TEST(test_magickenhanceimage);
  RUN_TEST(test_magickequalizeimage);
  RUN_TEST(test_magickevaluateimage);
  RUN_TEST(test_magickflattenimages);
  RUN_TEST(test_magickflipimage);
  RUN_TEST(test_magickflopimage);
  RUN_TEST(test_magickframeimage);
  RUN_TEST(test_magickfximage);
  RUN_TEST(test_magickgammaimage);
  RUN_TEST(test_magickgaussianblurimage);
  RUN_TEST(test_magickgetcharheight);
  RUN_TEST(test_magickgetcharwidth);
  RUN_TEST(test_magickgetexception);
  RUN_TEST(test_magickgetexceptionstring);
  RUN_TEST(test_magickgetexceptiontype);
  RUN_TEST(test_magickgetfilename);
  RUN_TEST(test_magickgetformat);
  RUN_TEST(test_magickgetimage);
  RUN_TEST(test_magickgetimagebackgroundcolor);
  RUN_TEST(test_magickgetimageblob);
  RUN_TEST(test_magickgetimageblueprimary);
  RUN_TEST(test_magickgetimagebordercolor);
  RUN_TEST(test_magickgetimagechannelmean);
  RUN_TEST(test_magickgetimagecolormapcolor);
  RUN_TEST(test_magickgetimagecolors);
  RUN_TEST(test_magickgetimagecolorspace);
  RUN_TEST(test_magickgetimagecompose);
  RUN_TEST(test_magickgetimagecompression);
  RUN_TEST(test_magickgetimagecompressionquality);
  RUN_TEST(test_magickgetimagedelay);
  RUN_TEST(test_magickgetimagedepth);
  RUN_TEST(test_magickgetimagedispose);
  RUN_TEST(test_magickgetimageextrema);
  RUN_TEST(test_magickgetimagefilename);
  RUN_TEST(test_magickgetimageformat);
  RUN_TEST(test_magickgetimagegamma);
  RUN_TEST(test_magickgetimagegreenprimary);
  RUN_TEST(test_magickgetimageheight);
  RUN_TEST(test_magickgetimagehistogram);
  RUN_TEST(test_magickgetimageindex);
  RUN_TEST(test_magickgetimageinterlacescheme);
  RUN_TEST(test_magickgetimageiterations);
  RUN_TEST(test_magickgetimagemattecolor);
  RUN_TEST(test_magickgetimagemimetype);
  RUN_TEST(test_magickgetimagepixels);
  RUN_TEST(test_magickgetimageprofile);
  RUN_TEST(test_magickgetimageredprimary);
  RUN_TEST(test_magickgetimagerenderingintent);
  RUN_TEST(test_magickgetimageresolution);
  RUN_TEST(test_magickgetimagescene);
  RUN_TEST(test_magickgetimagesignature);
  RUN_TEST(test_magickgetimagesize);
  RUN_TEST(test_magickgetimagetype);
  RUN_TEST(test_magickgetimageunits);
  RUN_TEST(test_magickgetimagevirtualpixelmethod);
  RUN_TEST(test_magickgetimagewhitepoint);
  RUN_TEST(test_magickgetimagewidth);
  RUN_TEST(test_magickgetimagesblob);
  RUN_TEST(test_magickgetinterlacescheme);
  RUN_TEST(test_magickgetmaxtextadvance);
  RUN_TEST(test_magickgetmimetype);
  RUN_TEST(test_magickgetnumberimages);
  RUN_TEST(test_magickgetsamplingfactors);
  RUN_TEST(test_magickgetsize);
  RUN_TEST(test_magickgetstringheight);
  RUN_TEST(test_magickgetstringwidth);
  RUN_TEST(test_magickgettextascent);
  RUN_TEST(test_magickgettextdescent);
  RUN_TEST(test_magickgetwandsize);
  RUN_TEST(test_magickhasnextimage);
  RUN_TEST(test_magickhaspreviousimage);
  RUN_TEST(test_magickimplodeimage);
  RUN_TEST(test_magicklabelimage);
  RUN_TEST(test_magicklevelimage);
  RUN_TEST(test_magickmagnifyimage);
  RUN_TEST(test_magickmapimage);
  RUN_TEST(test_magickmattefloodfillimage);
  RUN_TEST(test_magickmedianfilterimage);
  RUN_TEST(test_magickminifyimage);
  RUN_TEST(test_magickmodulateimage);
  RUN_TEST(test_magickmontageimage);
  RUN_TEST(test_magickmorphimages);
  RUN_TEST(test_magickmosaicimages);
  RUN_TEST(test_magickmotionblurimage);
  RUN_TEST(test_magicknegateimage);
  RUN_TEST(test_magicknewimage);
  RUN_TEST(test_magicknextimage);
  RUN_TEST(test_magicknormalizeimage);
  RUN_TEST(test_magickoilpaintimage);
  RUN_TEST(test_magickpaintopaqueimage);
  RUN_TEST(test_magickpainttransparentimage);
  RUN_TEST(test_magickpingimage);
  RUN_TEST(test_magickposterizeimage);
  RUN_TEST(test_magickpreviewimages);
  RUN_TEST(test_magickpreviousimage);
  RUN_TEST(test_magickprofileimage);
  RUN_TEST(test_magickquantizeimage);
  RUN_TEST(test_magickquantizeimages);
  RUN_TEST(test_magickqueryfontmetrics);
  RUN_TEST(test_magickradialblurimage);
  RUN_TEST(test_magickraiseimage);
  RUN_TEST(test_magickreadimage);
  RUN_TEST(test_magickreadimageblob);
  RUN_TEST(test_magickreadimagefile);
  RUN_TEST(test_magickreadimages);
  RUN_TEST(test_magickreducenoiseimage);
  RUN_TEST(test_magickremoveimage);
  RUN_TEST(test_magickremoveimageprofile);
  RUN_TEST(test_magickremoveimageprofiles);
  RUN_TEST(test_magickresampleimage);
  RUN_TEST(test_magickresetiterator);
  RUN_TEST(test_magickresizeimage);
  RUN_TEST(test_magickrollimage);
  RUN_TEST(test_magickrotateimage);
  RUN_TEST(test_magicksampleimage);
  RUN_TEST(test_magickscaleimage);
  RUN_TEST(test_magickseparateimagechannel);
  RUN_TEST(test_magicksetcompressionquality);
  RUN_TEST(test_magicksetfilename);
  RUN_TEST(test_magicksetfirstiterator);
  RUN_TEST(test_magicksetformat);
  RUN_TEST(test_magicksetimage);
  RUN_TEST(test_magicksetimagebackgroundcolor);
  RUN_TEST(test_magicksetimagebias);
  RUN_TEST(test_magicksetimageblueprimary);
  RUN_TEST(test_magicksetimagebordercolor);
  RUN_TEST(test_magicksetimagecolormapcolor);
  RUN_TEST(test_magicksetimagecolorspace);
  RUN_TEST(test_magicksetimagecompose);
  RUN_TEST(test_magicksetimagecompression);
  RUN_TEST(test_magicksetimagecompressionquality);
  RUN_TEST(test_magicksetimagedelay);
  RUN_TEST(test_magicksetimagedepth);
  RUN_TEST(test_magicksetimagedispose);
  RUN_TEST(test_magicksetimagefilename);
  RUN_TEST(test_magicksetimageformat);
  RUN_TEST(test_magicksetimagegamma);
  RUN_TEST(test_magicksetimagegreenprimary);
  RUN_TEST(test_magicksetimageindex);
  RUN_TEST(test_magicksetimageinterlacescheme);
  RUN_TEST(test_magicksetimageiterations);
  RUN_TEST(test_magicksetimagemattecolor);
  RUN_TEST(test_magicksetimageoption);
  RUN_TEST(test_magicksetimagepixels);
  RUN_TEST(test_magicksetimageprofile);
  RUN_TEST(test_magicksetimageredprimary);
  RUN_TEST(test_magicksetimagerenderingintent);
  RUN_TEST(test_magicksetimageresolution);
  RUN_TEST(test_magicksetimagescene);
  RUN_TEST(test_magicksetimagetype);
  RUN_TEST(test_magicksetimageunits);
  RUN_TEST(test_magicksetimagevirtualpixelmethod);
  RUN_TEST(test_magicksetimagewhitepoint);
  RUN_TEST(test_magicksetinterlacescheme);
  RUN_TEST(test_magicksetlastiterator);
  RUN_TEST(test_magicksetpassphrase);
  RUN_TEST(test_magicksetresolution);
  RUN_TEST(test_magicksetsamplingfactors);
  RUN_TEST(test_magicksetsize);
  RUN_TEST(test_magicksetwandsize);
  RUN_TEST(test_magicksharpenimage);
  RUN_TEST(test_magickshaveimage);
  RUN_TEST(test_magickshearimage);
  RUN_TEST(test_magicksolarizeimage);
  RUN_TEST(test_magickspliceimage);
  RUN_TEST(test_magickspreadimage);
  RUN_TEST(test_magicksteganoimage);
  RUN_TEST(test_magickstereoimage);
  RUN_TEST(test_magickstripimage);
  RUN_TEST(test_magickswirlimage);
  RUN_TEST(test_magicktextureimage);
  RUN_TEST(test_magickthresholdimage);
  RUN_TEST(test_magicktintimage);
  RUN_TEST(test_magicktransformimage);
  RUN_TEST(test_magicktrimimage);
  RUN_TEST(test_magickunsharpmaskimage);
  RUN_TEST(test_magickwaveimage);
  RUN_TEST(test_magickwhitethresholdimage);
  RUN_TEST(test_magickwriteimage);
  RUN_TEST(test_magickwriteimagefile);
  RUN_TEST(test_magickwriteimages);
  RUN_TEST(test_magickwriteimagesfile);
  RUN_TEST(test_pixelgetalpha);
  RUN_TEST(test_pixelgetalphaquantum);
  RUN_TEST(test_pixelgetblack);
  RUN_TEST(test_pixelgetblackquantum);
  RUN_TEST(test_pixelgetblue);
  RUN_TEST(test_pixelgetbluequantum);
  RUN_TEST(test_pixelgetcolorasstring);
  RUN_TEST(test_pixelgetcolorcount);
  RUN_TEST(test_pixelgetcyan);
  RUN_TEST(test_pixelgetcyanquantum);
  RUN_TEST(test_pixelgetexception);
  RUN_TEST(test_pixelgetexceptionstring);
  RUN_TEST(test_pixelgetexceptiontype);
  RUN_TEST(test_pixelgetgreen);
  RUN_TEST(test_pixelgetgreenquantum);
  RUN_TEST(test_pixelgetindex);
  RUN_TEST(test_pixelgetmagenta);
  RUN_TEST(test_pixelgetmagentaquantum);
  RUN_TEST(test_pixelgetopacity);
  RUN_TEST(test_pixelgetopacityquantum);
  RUN_TEST(test_pixelgetquantumcolor);
  RUN_TEST(test_pixelgetred);
  RUN_TEST(test_pixelgetredquantum);
  RUN_TEST(test_pixelgetyellow);
  RUN_TEST(test_pixelgetyellowquantum);
  RUN_TEST(test_pixelsetalpha);
  RUN_TEST(test_pixelsetalphaquantum);
  RUN_TEST(test_pixelsetblack);
  RUN_TEST(test_pixelsetblackquantum);
  RUN_TEST(test_pixelsetblue);
  RUN_TEST(test_pixelsetbluequantum);
  RUN_TEST(test_pixelsetcolor);
  RUN_TEST(test_pixelsetcolorcount);
  RUN_TEST(test_pixelsetcyan);
  RUN_TEST(test_pixelsetcyanquantum);
  RUN_TEST(test_pixelsetgreen);
  RUN_TEST(test_pixelsetgreenquantum);
  RUN_TEST(test_pixelsetindex);
  RUN_TEST(test_pixelsetmagenta);
  RUN_TEST(test_pixelsetmagentaquantum);
  RUN_TEST(test_pixelsetopacity);
  RUN_TEST(test_pixelsetopacityquantum);
  RUN_TEST(test_pixelsetquantumcolor);
  RUN_TEST(test_pixelsetred);
  RUN_TEST(test_pixelsetredquantum);
  RUN_TEST(test_pixelsetyellow);
  RUN_TEST(test_pixelsetyellowquantum);
  RUN_TEST(test_pixelgetiteratorexception);
  RUN_TEST(test_pixelgetiteratorexceptionstring);
  RUN_TEST(test_pixelgetiteratorexceptiontype);
  RUN_TEST(test_pixelgetnextiteratorrow);
  RUN_TEST(test_pixelresetiterator);
  RUN_TEST(test_pixelsetiteratorrow);
  RUN_TEST(test_pixelsynciterator);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtMagick::test_magickgetcopyright() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgethomeurl() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetpackagename() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetquantumdepth() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetreleasedate() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetresourcelimit() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetversion() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetversionnumber() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetversionstring() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickqueryconfigureoption() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickqueryconfigureoptions() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickqueryfonts() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickqueryformats() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetresourcelimit() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_newdrawingwand() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_newmagickwand() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_newpixeliterator() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_newpixelregioniterator() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_newpixelwand() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_newpixelwandarray() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_newpixelwands() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_destroydrawingwand() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_destroymagickwand() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_destroypixeliterator() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_destroypixelwand() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_destroypixelwandarray() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_destroypixelwands() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_isdrawingwand() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_ismagickwand() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_ispixeliterator() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_ispixelwand() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_cleardrawingwand() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_clearmagickwand() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_clearpixeliterator() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_clearpixelwand() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_clonedrawingwand() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_clonemagickwand() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_wandgetexception() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_wandgetexceptionstring() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_wandgetexceptiontype() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_wandhasexception() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawaffine() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawannotation() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawarc() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawbezier() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawcircle() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawcolor() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawcomment() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawcomposite() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawellipse() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetclippath() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetcliprule() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetclipunits() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetexception() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetexceptionstring() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetexceptiontype() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetfillalpha() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetfillcolor() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetfillopacity() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetfillrule() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetfont() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetfontfamily() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetfontsize() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetfontstretch() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetfontstyle() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetfontweight() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetgravity() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetstrokealpha() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetstrokeantialias() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetstrokecolor() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetstrokedasharray() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetstrokedashoffset() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetstrokelinecap() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetstrokelinejoin() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetstrokemiterlimit() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetstrokeopacity() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetstrokewidth() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgettextalignment() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgettextantialias() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgettextdecoration() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgettextencoding() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgettextundercolor() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawgetvectorgraphics() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawline() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawmatte() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpathclose() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpathcurvetoabsolute() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpathcurvetoquadraticbezierabsolute() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpathcurvetoquadraticbezierrelative() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpathcurvetoquadraticbeziersmoothabsolute() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpathcurvetoquadraticbeziersmoothrelative() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpathcurvetorelative() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpathcurvetosmoothabsolute() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpathcurvetosmoothrelative() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpathellipticarcabsolute() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpathellipticarcrelative() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpathfinish() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpathlinetoabsolute() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpathlinetohorizontalabsolute() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpathlinetohorizontalrelative() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpathlinetorelative() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpathlinetoverticalabsolute() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpathlinetoverticalrelative() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpathmovetoabsolute() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpathmovetorelative() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpathstart() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpoint() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpolygon() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpolyline() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawrectangle() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawrender() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawrotate() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawroundrectangle() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawscale() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetclippath() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetcliprule() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetclipunits() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetfillalpha() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetfillcolor() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetfillopacity() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetfillpatternurl() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetfillrule() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetfont() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetfontfamily() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetfontsize() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetfontstretch() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetfontstyle() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetfontweight() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetgravity() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetstrokealpha() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetstrokeantialias() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetstrokecolor() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetstrokedasharray() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetstrokedashoffset() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetstrokelinecap() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetstrokelinejoin() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetstrokemiterlimit() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetstrokeopacity() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetstrokepatternurl() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetstrokewidth() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsettextalignment() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsettextantialias() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsettextdecoration() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsettextencoding() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsettextundercolor() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetvectorgraphics() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawsetviewbox() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawskewx() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawskewy() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawtranslate() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pushdrawingwand() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpushclippath() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpushdefs() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpushpattern() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_popdrawingwand() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpopclippath() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpopdefs() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_drawpoppattern() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickadaptivethresholdimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickaddimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickaddnoiseimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickaffinetransformimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickannotateimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickappendimages() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickaverageimages() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickblackthresholdimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickblurimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickborderimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickcharcoalimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickchopimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickclipimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickclippathimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickcoalesceimages() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickcolorfloodfillimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickcolorizeimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickcombineimages() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickcommentimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickcompareimages() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickcompositeimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickconstituteimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickcontrastimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickconvolveimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickcropimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickcyclecolormapimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickdeconstructimages() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickdescribeimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickdespeckleimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickdrawimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickechoimageblob() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickechoimagesblob() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickedgeimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickembossimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickenhanceimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickequalizeimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickevaluateimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickflattenimages() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickflipimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickflopimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickframeimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickfximage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgammaimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgaussianblurimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetcharheight() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetcharwidth() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetexception() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetexceptionstring() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetexceptiontype() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetfilename() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetformat() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagebackgroundcolor() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimageblob() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimageblueprimary() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagebordercolor() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagechannelmean() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagecolormapcolor() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagecolors() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagecolorspace() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagecompose() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagecompression() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagecompressionquality() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagedelay() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagedepth() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagedispose() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimageextrema() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagefilename() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimageformat() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagegamma() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagegreenprimary() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimageheight() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagehistogram() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimageindex() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimageinterlacescheme() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimageiterations() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagemattecolor() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagemimetype() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagepixels() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimageprofile() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimageredprimary() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagerenderingintent() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimageresolution() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagescene() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagesignature() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagesize() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagetype() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimageunits() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagevirtualpixelmethod() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagewhitepoint() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagewidth() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetimagesblob() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetinterlacescheme() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetmaxtextadvance() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetmimetype() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetnumberimages() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetsamplingfactors() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetsize() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetstringheight() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetstringwidth() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgettextascent() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgettextdescent() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickgetwandsize() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickhasnextimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickhaspreviousimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickimplodeimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicklabelimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicklevelimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickmagnifyimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickmapimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickmattefloodfillimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickmedianfilterimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickminifyimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickmodulateimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickmontageimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickmorphimages() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickmosaicimages() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickmotionblurimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicknegateimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicknewimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicknextimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicknormalizeimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickoilpaintimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickpaintopaqueimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickpainttransparentimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickpingimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickposterizeimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickpreviewimages() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickpreviousimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickprofileimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickquantizeimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickquantizeimages() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickqueryfontmetrics() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickradialblurimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickraiseimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickreadimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickreadimageblob() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickreadimagefile() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickreadimages() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickreducenoiseimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickremoveimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickremoveimageprofile() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickremoveimageprofiles() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickresampleimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickresetiterator() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickresizeimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickrollimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickrotateimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksampleimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickscaleimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickseparateimagechannel() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetcompressionquality() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetfilename() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetfirstiterator() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetformat() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimagebackgroundcolor() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimagebias() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimageblueprimary() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimagebordercolor() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimagecolormapcolor() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimagecolorspace() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimagecompose() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimagecompression() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimagecompressionquality() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimagedelay() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimagedepth() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimagedispose() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimagefilename() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimageformat() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimagegamma() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimagegreenprimary() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimageindex() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimageinterlacescheme() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimageiterations() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimagemattecolor() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimageoption() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimagepixels() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimageprofile() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimageredprimary() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimagerenderingintent() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimageresolution() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimagescene() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimagetype() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimageunits() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimagevirtualpixelmethod() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetimagewhitepoint() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetinterlacescheme() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetlastiterator() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetpassphrase() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetresolution() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetsamplingfactors() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetsize() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksetwandsize() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksharpenimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickshaveimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickshearimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksolarizeimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickspliceimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickspreadimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicksteganoimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickstereoimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickstripimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickswirlimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicktextureimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickthresholdimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicktintimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicktransformimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magicktrimimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickunsharpmaskimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickwaveimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickwhitethresholdimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickwriteimage() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickwriteimagefile() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickwriteimages() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_magickwriteimagesfile() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetalpha() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetalphaquantum() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetblack() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetblackquantum() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetblue() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetbluequantum() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetcolorasstring() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetcolorcount() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetcyan() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetcyanquantum() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetexception() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetexceptionstring() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetexceptiontype() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetgreen() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetgreenquantum() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetindex() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetmagenta() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetmagentaquantum() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetopacity() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetopacityquantum() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetquantumcolor() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetred() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetredquantum() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetyellow() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetyellowquantum() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsetalpha() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsetalphaquantum() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsetblack() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsetblackquantum() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsetblue() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsetbluequantum() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsetcolor() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsetcolorcount() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsetcyan() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsetcyanquantum() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsetgreen() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsetgreenquantum() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsetindex() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsetmagenta() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsetmagentaquantum() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsetopacity() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsetopacityquantum() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsetquantumcolor() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsetred() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsetredquantum() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsetyellow() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsetyellowquantum() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetiteratorexception() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetiteratorexceptionstring() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetiteratorexceptiontype() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelgetnextiteratorrow() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelresetiterator() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsetiteratorrow() {
  //VCB("<?php ");
  return true;
}

bool TestExtMagick::test_pixelsynciterator() {
  //VCB("<?php ");
  return true;
}
