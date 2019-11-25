set(BROTLI_ENC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/enc")
set(BROTLI_DEC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/dec")

auto_sources(enc_ccfiles "*.cc" "RECURSE" "${BROTLI_ENC_DIR}")
auto_sources(enc_cfiles "*.c" "RECURSE" "${BROTLI_ENC_DIR}")
auto_sources(enc_hfiles "*.h" "RECURSE" "${BROTLI_ENC_DIR}")

auto_sources(dec_ccfiles "*.cc" "RECURSE" "${BROTLI_DEC_DIR}")
auto_sources(dec_cfiles "*.c" "RECURSE" "${BROTLI_DEC_DIR}")
auto_sources(dec_hfiles "*.h" "RECURSE" "${BROTLI_DEC_DIR}")

add_library(brotli_enc STATIC ${enc_ccfiles} ${enc_cfiles} ${enc_hfiles})
auto_source_group(brotli_enc ${BROTLI_ENC_DIR} ${enc_ccfiles} ${enc_cfiles} ${enc_hfiles})
target_include_directories(brotli_enc PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_library(brotli_dec STATIC ${dec_ccfiles} ${dec_cfiles} ${dnc_hfiles})
auto_source_group(brotli_dec ${BROTLI_DEC_DIR} ${dec_ccfiles} ${dec_cfiles} ${dec_hfiles})
target_include_directories(brotli_dec PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
