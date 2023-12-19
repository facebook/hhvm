<?hh

abstract final class Php7BackportedExtOpensslCipherTestsInc {
  public static function getPHPOpenSSLCipherTests() {
		return dict[
			'aes-128-ccm' => vec[
				dict[
					'key' => '8c89a2ebc96c7602707fcf24b32d3833',
					'iv'  => '078ef822734701f670a55a0fe3',
					'aad' => '88c2712a9ddf11db8ef8227347015914' .
							'0dd646a200000700',
					'tag' => '31fc88004f35ee3d',
					'pt'  => '4fad2b1c290fa5ebd872fbc3f3a07489' .
								'8f8b2fbb',
					'ct'  => '9d59b15f371448c230f4d739052e13ab' .
								'3b1a7b10',
				],
				dict[
					'key' => 'f71eea4e1f58804b9717230ad0614641',
					'iv'  => '0dbff943b9f9a66b81eca48989',
					'aad' => '88425af28430fdabbff943b9f9a6ab1d' .
							'98c7fe7300000d00',
					'tag' => '309a8d5c466bbb71',
					'pt'  => 'abfda22d3a0bfc9cc1fc079363c2fca1' .
								'43e6eb1d',
					'ct'  => '9a709b60a39d40b1dfb612e18b5f114b' .
								'adb6cc86',
				],
			],
			'aes-256-ccm' => vec[
				dict[
					'key' => '1bde3251d41a8b5ea013c195ae128b21' .
							'8b3e0306376357077ef1c1c78548b92e',
					'iv'  => '5b8e40746f6b98e00f1d13ff41',
					'aad' => 'c17a32514eb6103f3249e076d4c871dc' .
							'97e04b286699e54491dc18f6d734d4c0',
					'tag' => '2024931d73bca480c24a24ece6b6c2bf',
					'pt'  => '53bd72a97089e312422bf72e242377b3' .
							'c6ee3e2075389b999c4ef7f28bd2b80a',
					'ct'  => '9a5fcccdb4cf04e7293d2775cc76a488' .
							'f042382d949b43b7d6bb2b9864786726',
				],
			],
			'aes-128-gcm' => vec[
				dict[
					'key' => '00000000000000000000000000000000',
					'iv'  => '000000000000000000000000',
					'tag' => '58e2fccefa7e3061367f1d57a4e7455a',
					'pt'  => '',
					'ct'  => '',
				],
				dict[
					'key' => '00000000000000000000000000000000',
					'iv'  => '000000000000000000000000',
					'tag' => 'ab6e47d42cec13bdf53a67b21257bddf',
					'pt'  => '00000000000000000000000000000000',
					'ct'  => '0388dace60b6a392f328c2b971b2fe78',
				],
				dict[
					'key' => 'feffe9928665731c6d6a8f9467308308',
					'iv'  => 'cafebabefacedbaddecaf888',
					'tag' => '4d5c2af327cd64a62cf35abd2ba6fab4',
					'pt'  => 'd9313225f88406e5a55909c5aff5269a' .
							'86a7a9531534f7da2e4c303d8a318a72' .
							'1c3c0c95956809532fcf0e2449a6b525' .
							'b16aedf5aa0de657ba637b391aafd255',
					'ct'  => '42831ec2217774244b7221b784d0d49c' .
							'e3aa212f2c02a4e035c17e2329aca12e' .
							'21d514b25466931c7d8f6a5aac84aa05' .
							'1ba30b396a0aac973d58e091473f5985',
				],
				dict[
					'key' => 'feffe9928665731c6d6a8f9467308308',
					'iv'  => 'cafebabefacedbaddecaf888',
					'aad' => 'feedfacedeadbeeffeedfacedeadbeefabaddad2',
					'tag' => '5bc94fbc3221a5db94fae95ae7121a47',
					'pt'  => 'd9313225f88406e5a55909c5aff5269a' .
							'86a7a9531534f7da2e4c303d8a318a72' .
							'1c3c0c95956809532fcf0e2449a6b525' .
							'b16aedf5aa0de657ba637b39',
					'ct'  => '42831ec2217774244b7221b784d0d49c' .
							'e3aa212f2c02a4e035c17e2329aca12e' .
							'21d514b25466931c7d8f6a5aac84aa05' .
							'1ba30b396a0aac973d58e091',
				],
				dict[
					'key' => 'feffe9928665731c6d6a8f9467308308',
					'iv'  => 'cafebabefacedbad',
					'aad' => 'feedfacedeadbeeffeedfacedeadbeefabaddad2',
					'tag' => '3612d2e79e3b0785561be14aaca2fccb',
					'pt'  => 'd9313225f88406e5a55909c5aff5269a' .
							'86a7a9531534f7da2e4c303d8a318a72' .
							'1c3c0c95956809532fcf0e2449a6b525' .
							'b16aedf5aa0de657ba637b39',
					'ct'  => '61353b4c2806934a777ff51fa22a4755' .
							'699b2a714fcdc6f83766e5f97b6c7423' .
							'73806900e49f24b22b097544d4896b42' .
							'4989b5e1ebac0f07c23f4598'
				],
				dict[
					'key' => 'feffe9928665731c6d6a8f9467308308',
					'iv'  => '9313225df88406e555909c5aff5269aa' .
							'6a7a9538534f7da1e4c303d2a318a728' .
							'c3c0c95156809539fcf0e2429a6b5254' .
							'16aedbf5a0de6a57a637b39b',
					'aad' => 'feedfacedeadbeeffeedfacedeadbeefabaddad2',
					'tag' => '619cc5aefffe0bfa462af43c1699d050',
					'pt'  => 'd9313225f88406e5a55909c5aff5269a' .
							'86a7a9531534f7da2e4c303d8a318a72' .
							'1c3c0c95956809532fcf0e2449a6b525' .
							'b16aedf5aa0de657ba637b39',
					'ct'  => '8ce24998625615b603a033aca13fb894' .
							'be9112a5c3a211a8ba262a3cca7e2ca7' .
							'01e4a9a4fba43c90ccdcb281d48c7c6f' .
							'd62875d2aca417034c34aee5',
				],
			],
			'aes-192-gcm' => vec[
				dict[
					'key' => 'feffe9928665731c6d6a8f9467308308' .
								'feffe9928665731c',
					'iv'  => 'cafebabefacedbaddecaf888',
					'aad' => 'feedfacedeadbeeffeedfacedeadbeef' .
								'abaddad2',
					'tag' => '2519498e80f1478f37ba55bd6d27618c',
					'pt'  => 'd9313225f88406e5a55909c5aff5269a' .
							'86a7a9531534f7da2e4c303d8a318a72' .
							'1c3c0c95956809532fcf0e2449a6b525' .
							'b16aedf5aa0de657ba637b39',
					'ct'  => '3980ca0b3c00e841eb06fac4872a2757' .
							'859e1ceaa6efd984628593b40ca1e19c' .
							'7d773d00c144c525ac619d18c84a3f47' .
							'18e2448b2fe324d9ccda2710',
				],
				dict[
					'key' => 'feffe9928665731c6d6a8f9467308308' .
								'feffe9928665731c',
					'iv'  => 'cafebabefacedbad',
					'aad' => 'feedfacedeadbeeffeedfacedeadbeef' .
								'abaddad2',
					'tag' => '65dcc57fcf623a24094fcca40d3533f8',
					'pt'  => 'd9313225f88406e5a55909c5aff5269a' .
							'86a7a9531534f7da2e4c303d8a318a72' .
							'1c3c0c95956809532fcf0e2449a6b525' .
							'b16aedf5aa0de657ba637b39',
					'ct'  => '0f10f599ae14a154ed24b36e25324db8' .
							'c566632ef2bbb34f8347280fc4507057' .
							'fddc29df9a471f75c66541d4d4dad1c9' .
							'e93a19a58e8b473fa0f062f7',
				],
			],
			'aes-256-gcm' => vec[
				dict[
					'key' => '00000000000000000000000000000000' .
								'00000000000000000000000000000000',
					'iv'  => '000000000000000000000000',
					'tag' => 'd0d1c8a799996bf0265b98b5d48ab919',
					'pt'  => '00000000000000000000000000000000',
					'ct'  => 'cea7403d4d606b6e074ec5d3baf39d18',
				],
				dict[
					'key' => 'feffe9928665731c6d6a8f9467308308' .
								'feffe9928665731c6d6a8f9467308308',
					'iv'  => 'cafebabefacedbaddecaf888',
					'tag' => 'b094dac5d93471bdec1a502270e3cc6c',
					'pt'  => 'd9313225f88406e5a55909c5aff5269a' .
							'86a7a9531534f7da2e4c303d8a318a72' .
							'1c3c0c95956809532fcf0e2449a6b525' .
							'b16aedf5aa0de657ba637b391aafd255',
					'ct'  => '522dc1f099567d07f47f37a32a84427d' .
							'643a8cdcbfe5c0c97598a2bd2555d1aa' .
							'8cb08e48590dbb3da7b08b1056828838' .
							'c5f61e6393ba7a0abcc9f662898015ad',
				],
				dict[
					'key' => 'feffe9928665731c6d6a8f9467308308' .
								'feffe9928665731c6d6a8f9467308308',
					'iv'  => 'cafebabefacedbaddecaf888',
					'aad' => 'feedfacedeadbeeffeedfacedeadbeef' .
								'abaddad2',
					'tag' => '76fc6ece0f4e1768cddf8853bb2d551b',
					'pt'  => 'd9313225f88406e5a55909c5aff5269a' .
							'86a7a9531534f7da2e4c303d8a318a72' .
							'1c3c0c95956809532fcf0e2449a6b525' .
							'b16aedf5aa0de657ba637b39',
					'ct'  => '522dc1f099567d07f47f37a32a84427d' .
							'643a8cdcbfe5c0c97598a2bd2555d1aa' .
							'8cb08e48590dbb3da7b08b1056828838' .
							'c5f61e6393ba7a0abcc9f662',
				],
			]
		];
	}
}

function openssl_get_cipher_tests($method)
{


	$tests = vec[];

	foreach (Php7BackportedExtOpensslCipherTestsInc::getPHPOpenSSLCipherTests()[$method] as $instance) {
		$test = dict[];
		foreach ($instance as $field_name => $field_value) {
			$test[$field_name] = pack("H*", $field_value);
		}
		if (!isset($test['aad'])) {
			$test['aad'] = "";
		}
		$tests[] = $test;
	}

	return $tests;
}
