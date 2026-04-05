{
  "targets": [
    {
      "target_name": "addon",
      "sources": [
        "addon.cc",
        "s63lib/s63/blowfish.cpp",
        "s63lib/s63/s63.cpp",
        "s63lib/s63/s63client.cpp",
        "s63lib/s63/simple_zip.cpp",
        "s63lib/s63/zlib/adler32.c",
        "s63lib/s63/zlib/compress.c",
        "s63lib/s63/zlib/crc32.c",
        "s63lib/s63/zlib/deflate.c",
        "s63lib/s63/zlib/gzclose.c",
        "s63lib/s63/zlib/gzlib.c",
        "s63lib/s63/zlib/gzread.c",
        "s63lib/s63/zlib/gzwrite.c",
        "s63lib/s63/zlib/infback.c",
        "s63lib/s63/zlib/inffast.c",
        "s63lib/s63/zlib/inflate.c",
        "s63lib/s63/zlib/inftrees.c",
        "s63lib/s63/zlib/trees.c",
        "s63lib/s63/zlib/uncompr.c",
        "s63lib/s63/zlib/zutil.c"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "s63lib/s63"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "defines": [
        "NAPI_CPP_EXCEPTIONS"
      ],
      "cflags": [ "-fexceptions" ],
      "cflags_cc": [ "-fexceptions", "-std=c++17" ],
      "xcode_settings": {
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "CLANG_CXX_LANGUAGE_STANDARD": "c++17"
      },
      "msvs_settings": {
        "VCCLCompilerTool": {
          "ExceptionHandling": 1,
          "AdditionalOptions": [ "/std:c++17" ]
        }
      }
    }
  ]
}