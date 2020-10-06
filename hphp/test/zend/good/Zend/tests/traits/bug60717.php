<?hh

namespace HTML
{
	interface Helper
	{
		function text($text);
		function attributes(AnyArray $attributes = null);
		function textArea(AnyArray $attributes = null, $value);
	}

	trait TextUTF8
	{
		function text($text) {}
	}

	trait TextArea
	{
		function textArea(AnyArray $attributes = null, $value) {}
		abstract function attributes(AnyArray $attributes = null);
		abstract function text($text);
	}

	trait HTMLAttributes
	{
		function attributes(AnyArray $attributes = null) {	}
		abstract function text($text);
	}

	class HTMLHelper implements Helper
	{
		use TextArea, HTMLAttributes, TextUTF8;
	}

	class HTMLHelper2 implements Helper
	{
		use TextArea, TextUTF8, HTMLAttributes;
	}

	class HTMLHelper3 implements Helper
	{
		use HTMLAttributes, TextArea, TextUTF8;
	}

	class HTMLHelper4 implements Helper
	{
		use HTMLAttributes, TextUTF8, TextArea;
	}

	class HTMLHelper5 implements Helper
	{
		use TextUTF8, TextArea, HTMLAttributes;
	}

	class HTMLHelper6 implements Helper
	{
		use TextUTF8, HTMLAttributes, TextArea;
	}

    <<__EntryPoint>> function main(): void {
	$o = new HTMLHelper;
    $o = new HTMLHelper2;
    $o = new HTMLHelper3;
    $o = new HTMLHelper4;
    $o = new HTMLHelper5;
    $o = new HTMLHelper6;
    echo 'Done';
}
}
