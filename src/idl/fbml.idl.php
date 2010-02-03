<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('fbml_tag_list_expanded_11', Boolean);

f('fbml_complex_expand_tag_list_11', NULL,
  array('new_tags' => VariantMap,
        'new_attrs' => VariantMap,
        'special_html' => VariantMap,
        'precache' => VariantMap,
        'style' => VariantMap,
        'style_attrs' => VariantMap,
        'script_attrs' => VariantMap,
        'rewrite_attrs' => VariantMap,
        'special_attrs' => VariantMap,
        'schema' => VariantMap));

f('fbml_parse_opaque_11', VariantMap,
  array('unsanitized_fbml' => String,
        'body_only' => Boolean,
        'preserve_comment' => Boolean,
        'internal_mode' => array(Boolean, 'false'),
        'css_sanitizer' => array(Variant, 'null_variant'),
        'js_sanitizer' => array(Variant, 'null_variant'),
        'attr_rewriter' => array(Variant, 'null_variant')));

f('fbml_sanitize_css_11', VariantMap,
  array('unsanitized_css' => String,
        'decl_only' => Boolean,
        'line_number' => Int32,
        'css_sanitizer' => StringVec));

f('fbml_sanitize_js_11', VariantMap,
  array('unsanitized_js' => String,
        'line_number' => Int32,
        'js_sanitizer' => StringVec));

f('fbml_get_tag_name_11', String,
  array('node' => Resource));

f('fbml_get_children_11', VariantVec,
  array('node' => Resource));

f('fbml_get_children_count_11', Int32,
  array('node' => Resource));

f('fbml_get_children_by_name_11', VariantVec,
  array('node' => Resource,
        'tag' => String));

f('fbml_get_attributes_11', StringMap,
  array('node' => Resource));

f('fbml_get_attribute_11', String,
  array('node' => Resource,
        'name' => String));

f('fbml_attr_to_bool_11', Variant,
  array('name' => String));

f('fbml_attr_to_color_11', String,
  array('name' => String));

f('fbml_get_text_11', String,
  array('node' => Resource));

f('fbml_precache_11', NULL,
  array('node' => Resource,
        'data' => Variant,
        'callback' => String));

f('fbml_batch_precache_11', VariantMap,
  array('node' => Resource));

f('fbml_render_children_11', String,
  array('node' => Resource,
        'data' => Variant,
        'html_callback' => String,
        'fb_callback' => String,
        'internal_mode' => array(Int32, '0')));

f('fbml_flatten_11', String,
  array('node' => Resource));

f('html_profile', String,
  array('html' => String));

f('fbjsparse', String,
  array('input' => String));
