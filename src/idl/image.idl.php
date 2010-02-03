<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// gd

f('gd_info', VariantMap);

f('getimagesize', Variant,
  array('filename' => String,
        'imageinfo' => array(VariantMap | Reference, 'null')));

f('image_type_to_extension', String,
  array('imagetype' => Int32,
        'include_dot' => array(Boolean, 'true')));

f('image_type_to_mime_type', String,
  array('imagetype' => Int32));

f('image2wbmp', Boolean,
  array('image' => Resource,
        'filename' => array(String, 'null_string'),
        'threshold' => array(Int32, '-1')));

f('imagealphablending', Boolean,
  array('image' => Resource,
        'blendmode' => Boolean));

f('imageantialias', Boolean,
  array('image' => Resource,
        'on' => Boolean));

f('imagearc', Boolean,
  array('image' => Resource,
        'cx' => Int32,
        'cy' => Int32,
        'width' => Int32,
        'height' => Int32,
        'start' => Int32,
        'end' => Int32,
        'color' => Int32));

f('imagechar', Boolean,
  array('image' => Resource,
        'font' => Int32,
        'x' => Int32,
        'y' => Int32,
        'c' => String,
        'color' => Int32));

f('imagecharup', Boolean,
  array('image' => Resource,
        'font' => Int32,
        'x' => Int32,
        'y' => Int32,
        'c' => String,
        'color' => Int32));

f('imagecolorallocate', Variant,
  array('image' => Resource,
        'red' => Int32,
        'green' => Int32,
        'blue' => Int32));

f('imagecolorallocatealpha', Variant,
  array('image' => Resource,
        'red' => Int32,
        'green' => Int32,
        'blue' => Int32,
        'alpha' => Int32));

f('imagecolorat', Variant,
  array('image' => Resource,
        'x' => Int32,
        'y' => Int32));

f('imagecolorclosest', Variant,
  array('image' => Resource,
        'red' => Int32,
        'green' => Int32,
        'blue' => Int32));

f('imagecolorclosestalpha', Variant,
  array('image' => Resource,
        'red' => Int32,
        'green' => Int32,
        'blue' => Int32,
        'alpha' => Int32));

f('imagecolorclosesthwb', Variant,
  array('image' => Resource,
        'red' => Int32,
        'green' => Int32,
        'blue' => Int32));

f('imagecolordeallocate', Boolean,
  array('image' => Resource,
        'color' => Int32));

f('imagecolorexact', Variant,
  array('image' => Resource,
        'red' => Int32,
        'green' => Int32,
        'blue' => Int32));

f('imagecolorexactalpha', Variant,
  array('image' => Resource,
        'red' => Int32,
        'green' => Int32,
        'blue' => Int32,
        'alpha' => Int32));

f('imagecolormatch', Variant,
  array('image1' => Resource,
        'image2' => Resource));

f('imagecolorresolve', Variant,
  array('image' => Resource,
        'red' => Int32,
        'green' => Int32,
        'blue' => Int32));

f('imagecolorresolvealpha', Variant,
  array('image' => Resource,
        'red' => Int32,
        'green' => Int32,
        'blue' => Int32,
        'alpha' => Int32));

f('imagecolorset', Variant,
  array('image' => Resource,
        'index' => Int32,
        'red' => Int32,
        'green' => Int32,
        'blue' => Int32));

f('imagecolorsforindex', Variant,
  array('image' => Resource,
        'index' => Int32));

f('imagecolorstotal', Variant,
  array('image' => Resource));

f('imagecolortransparent', Variant,
  array('image' => Resource,
        'color' => array(Int32, '-1')));

f('imageconvolution', Boolean,
  array('image' => Resource,
        'matrix' => Int64Map,
        'div' => Double,
        'offset' => Double));

f('imagecopy', Boolean,
  array('dst_im' => Resource,
        'src_im' => Resource,
        'dst_x' => Int32,
        'dst_y' => Int32,
        'src_x' => Int32,
        'src_y' => Int32,
        'src_w' => Int32,
        'src_h' => Int32));

f('imagecopymerge', Boolean,
  array('dst_im' => Resource,
        'src_im' => Resource,
        'dst_x' => Int32,
        'dst_y' => Int32,
        'src_x' => Int32,
        'src_y' => Int32,
        'src_w' => Int32,
        'src_h' => Int32,
        'pct' => Int32));

f('imagecopymergegray', Boolean,
  array('dst_im' => Resource,
        'src_im' => Resource,
        'dst_x' => Int32,
        'dst_y' => Int32,
        'src_x' => Int32,
        'src_y' => Int32,
        'src_w' => Int32,
        'src_h' => Int32,
        'pct' => Int32));

f('imagecopyresampled', Boolean,
  array('dst_im' => Resource,
        'src_im' => Resource,
        'dst_x' => Int32,
        'dst_y' => Int32,
        'src_x' => Int32,
        'src_y' => Int32,
        'dst_w' => Int32,
        'dst_h' => Int32,
        'src_w' => Int32,
        'src_h' => Int32));

f('imagecopyresized', Boolean,
  array('dst_im' => Resource,
        'src_im' => Resource,
        'dst_x' => Int32,
        'dst_y' => Int32,
        'src_x' => Int32,
        'src_y' => Int32,
        'dst_w' => Int32,
        'dst_h' => Int32,
        'src_w' => Int32,
        'src_h' => Int32));

f('imagecreate', Variant,
  array('width' => Int32,
        'height' => Int32));

f('imagecreatefromgd2part', Variant,
  array('filename' => String,
        'srcX' => Int32,
        'srcY' => Int32,
        'width' => Int32,
        'height' => Int32));

f('imagecreatefromgd',     Variant,  array('filename' => String));
f('imagecreatefromgd2',    Variant,  array('filename' => String));
f('imagecreatefromgif',    Variant,  array('filename' => String));
f('imagecreatefromjpeg',   Variant,  array('filename' => String));
f('imagecreatefrompng',    Variant,  array('filename' => String));
f('imagecreatefromstring', Variant,  array('data' => String));
f('imagecreatefromwbmp',   Variant,  array('filename' => String));
f('imagecreatefromxbm',    Variant,  array('filename' => String));
f('imagecreatefromxpm',    Variant,  array('filename' => String));
f('imagecreatetruecolor',  Variant,
  array('width' => Int32,
        'height' => Int32));

f('imagedashedline', Boolean,
  array('image' => Resource,
        'x1' => Int32,
        'y1' => Int32,
        'x2' => Int32,
        'y2' => Int32,
        'color' => Int32));

f('imagedestroy', Boolean,
  array('image' => Resource));

f('imageellipse', Boolean,
  array('image' => Resource,
        'cx' => Int32,
        'cy' => Int32,
        'width' => Int32,
        'height' => Int32,
        'color' => Int32));

f('imagefill', Boolean,
  array('image' => Resource,
        'x' => Int32,
        'y' => Int32,
        'color' => Int32));

f('imagefilledarc', Boolean,
  array('image' => Resource,
        'cx' => Int32,
        'cy' => Int32,
        'width' => Int32,
        'height' => Int32,
        'start' => Int32,
        'end' => Int32,
        'color' => Int32,
        'style' => Int32));

f('imagefilledellipse', Boolean,
  array('image' => Resource,
        'cx' => Int32,
        'cy' => Int32,
        'width' => Int32,
        'height' => Int32,
        'color' => Int32));

f('imagefilledpolygon', Boolean,
  array('image' => Resource,
        'points' => VariantVec,
        'num_points' => Int32,
        'color' => Int32));

f('imagefilledrectangle', Boolean,
  array('image' => Resource,
        'x1' => Int32,
        'y1' => Int32,
        'x2' => Int32,
        'y2' => Int32,
        'color' => Int32));

f('imagefilltoborder', Boolean,
  array('image' => Resource,
        'x' => Int32,
        'y' => Int32,
        'border' => Int32,
        'color' => Int32));

f('imagefilter', Boolean,
  array('image' => Resource,
        'filtertype' => Int32,
        'arg1' => array(Int32, '0'),
        'arg2' => array(Int32, '0'),
        'arg3' => array(Int32, '0'),
        'arg4' => array(Int32, '0')));

f('imagefontheight', Int32,
  array('font' => Int32));

f('imagefontwidth', Int32,
  array('font' => Int32));

f('imageftbbox', Variant,
  array('size' => Double,
        'angle' => Double,
        'font_file' => String,
        'text' => String,
        'extrainfo' => array(VariantMap, 'null')));

f('imagefttext', Variant,
  array('image' => Resource,
        'size' => Double,
        'angle' => Double,
        'x' => Int32,
        'y' => Int32,
        'col' => Int32,
        'font_file' => String,
        'text' => String,
        'extrainfo' => array(VariantMap, 'null')));

f('imagegammacorrect', Boolean,
  array('image' => Resource,
        'inputgamma' => Double,
        'outputgamma' => Double));

f('imagegd2', Boolean,
  array('image' => Resource,
        'filename' => array(String, 'null_string'),
        'chunk_size' => array(Int32, '0'),
        'type' => array(Int32, '0')));

f('imagegd', Boolean,
  array('image' => Resource,
        'filename' => array(String, 'null_string')));

f('imagegif', Boolean,
  array('image' => Resource,
        'filename' => array(String, 'null_string')));

f('imagegrabscreen', Variant);

f('imagegrabwindow', Variant,
  array('window' => Int32,
        'client_area' => array(Int32, '0')));

f('imageinterlace', Variant,
  array('image' => Resource,
        'interlace' => array(Int32, '0')));

f('imageistruecolor', Boolean,
  array('image' => Resource));

f('imagejpeg', Boolean,
  array('image' => Resource,
        'filename' => array(String, 'null_string'),
        'quality' => array(Int32, '-1')));

f('imagelayereffect', Boolean,
  array('image' => Resource,
        'effect' => Int32));

f('imageline', Boolean,
  array('image' => Resource,
        'x1' => Int32,
        'y1' => Int32,
        'x2' => Int32,
        'y2' => Int32,
        'color' => Int32));

f('imageloadfont', Variant,
  array('file' => String));

f('imagepalettecopy', NULL,
  array('destination' => Resource,
        'source' => Resource));

f('imagepng', Boolean,
  array('image' => Resource,
        'filename' => array(String, 'null_string'),
        'quality' => array(Int32, '-1'),
        'filters' => array(Int32, '-1')));

f('imagepolygon', Boolean,
  array('image' => Resource,
        'points' => VariantVec,
        'num_points' => Int32,
        'color' => Int32));

f('imagepsbbox', Int64Vec,
  array('text' => String,
        'font' => Int32,
        'size' => Int32,
        'space' => array(Int32, '0'),
        'tightness' => array(Int32, '0'),
        'angle' => array(Double, '0.0')));

f('imagepsencodefont', Boolean,
  array('font_index' => Resource,
        'encodingfile' => String));

f('imagepsextendfont', Boolean,
  array('font_index' => Int32,
        'extend' => Double));

f('imagepsfreefont', Boolean,
  array('fontindex' => Resource));

f('imagepsloadfont', Resource,
  array('filename' => String));

f('imagepsslantfont', Boolean,
  array('font_index' => Resource,
        'slant' => Double));

f('imagepstext', StringVec,
  array('image' => Resource,
        'text' => String,
        'font' => Resource,
        'size' => Int32,
        'foreground' => Int32,
        'background' => Int32,
        'x' => Int32,
        'y' => Int32,
        'space' => array(Int32, '0'),
        'tightness' => array(Int32, '0'),
        'angle' => array(Double, '0.0'),
        'antialias_steps' => array(Int32, '0')));

f('imagerectangle', Boolean,
  array('image' => Resource,
        'x1' => Int32,
        'y1' => Int32,
        'x2' => Int32,
        'y2' => Int32,
        'color' => Int32));

f('imagerotate', Variant,
  array('source_image' => Resource,
        'angle' => Double,
        'bgd_color' => Int32,
        'ignore_transparent' => array(Int32, '0')));

f('imagesavealpha', Boolean,
  array('image' => Resource,
        'saveflag' => Boolean));

f('imagesetbrush', Boolean,
  array('image' => Resource,
        'brush' => Resource));

f('imagesetpixel', Boolean,
  array('image' => Resource,
        'x' => Int32,
        'y' => Int32,
        'color' => Int32));

f('imagesetstyle', Boolean,
  array('image' => Resource,
        'style' => VariantMap));

f('imagesetthickness', Boolean,
  array('image' => Resource,
        'thickness' => Int32));

f('imagesettile', Boolean,
  array('image' => Resource,
        'tile' => Resource));

f('imagestring', Boolean,
  array('image' => Resource,
        'font' => Int32,
        'x' => Int32,
        'y' => Int32,
        'str' => String,
        'color' => Int32));

f('imagestringup', Boolean,
  array('image' => Resource,
        'font' => Int32,
        'x' => Int32,
        'y' => Int32,
        'str' => String,
        'color' => Int32));

f('imagesx', Variant,
  array('image' => Resource));

f('imagesy', Variant,
  array('image' => Resource));

f('imagetruecolortopalette', Variant,
  array('image' => Resource,
        'dither' => Boolean,
        'ncolors' => Int32));

f('imagettfbbox', Variant,
  array('size' => Double,
        'angle' => Double,
        'fontfile' => String,
        'text' => String));

f('imagettftext', Variant,
  array('image' => Resource,
        'size' => Double,
        'angle' => Double,
        'x' => Int32,
        'y' => Int32,
        'color' => Int32,
        'fontfile' => String,
        'text' => String));

f('imagetypes', Int32);

f('imagewbmp', Boolean,
  array('image' => Resource,
        'filename' => array(String, 'null_string'),
        'foreground' => array(Int32, '-1')));

f('imagexbm', Boolean,
  array('image' => Resource,
        'filename' => array(String, 'null_string'),
        'foreground' => array(Int32, '-1')));

f('iptcembed', Variant,
  array('iptcdata' => String,
        'jpeg_file_name' => String,
        'spool' => array(Int32, '0')));

f('iptcparse', Variant,
  array('iptcblock' => String));

f('jpeg2wbmp', Boolean,
  array('jpegname' => String,
        'wbmpname' => String,
        'dest_height' => Int32,
        'dest_width' => Int32,
        'threshold' => Int32));

f('png2wbmp', Boolean,
  array('pngname' => String,
        'wbmpname' => String,
        'dest_height' => Int32,
        'dest_width' => Int32,
        'threshold' => Int32));

///////////////////////////////////////////////////////////////////////////////
// exif

f('exif_imagetype', Variant,
  array('filename' => String));

f('exif_read_data', Variant,
  array('filename' => String,
        'sections' => array(String, 'null_string'),
        'arrays' => array(Boolean, 'false'),
        'thumbnail' => array(Boolean, 'false')));

f('read_exif_data', Variant,
  array('filename' => String,
        'sections' => array(String, 'null_string'),
        'arrays' => array(Boolean, 'false'),
        'thumbnail' => array(Boolean, 'false')));

f('exif_tagname', Variant,
  array('index' => Int32));

f('exif_thumbnail', Variant,
  array('filename' => String,
        'width' => array(Int32 | Reference, 'null'),
        'height' => array(Int32 | Reference, 'null'),
        'imagetype' => array(Int32 | Reference, 'null')));
