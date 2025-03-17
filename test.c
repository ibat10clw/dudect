#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define DUDECT_IMPLEMENTATION
#include "src/dudect.h"

/* 原本的函式保留，但現在不再使用 */
int check_tag(uint8_t *x, uint8_t *y, size_t len) {
  return memcmp(x, y, len);
}

#define SECRET_LEN_BYTES (512)
uint8_t secret[SECRET_LEN_BYTES] = {0, 1, 2, 3, 4, 5, 6, 42};

/* 將測試目標改為使用 strdup 複製傳入的 C 字串 */
uint8_t do_one_computation(uint8_t *data) {
  char *buf = strdup((char *)data);
  return 1;
}

/* 定義新的字串最大長度與區塊大小（隨機字串大小介於 0~8 bytes，加上 '\0' 結尾） */
#define MAX_STR_LEN 8
#define CHUNK_SIZE (MAX_STR_LEN + 1)

/* 每次測量前準備輸入資料：
   - 依據隨機的 class 決定要產生空字串（class 為 0）
     或產生隨機長度（0~8 bytes）的字串（class 為 1） */
void prepare_inputs(dudect_config_t *c, uint8_t *input_data, uint8_t *classes) {
  for (size_t i = 0; i < c->number_measurements; i++) {
    classes[i] = randombit();
    if (classes[i] == 0) {
      /* 產生空字串 */
    } else {
      /* 產生隨機長度字串，長度介於 0 到 MAX_STR_LEN (含0) */
      uint8_t len;
      randombytes(&len, 1);
      len = len % (MAX_STR_LEN + 1);
      if (len > 0) {
        randombytes(input_data + i * c->chunk_size, len);
      }
      /* 加入 NUL 結尾 */
      input_data[i * c->chunk_size + len] = '\0';
      if (len < c->chunk_size - 1) {
        memset(input_data + i * c->chunk_size + len + 1, 0, c->chunk_size - len - 1);
      }
    }
  }
}

int run_test(void) {
  dudect_config_t config = {
      .chunk_size = CHUNK_SIZE,
#ifdef MEASUREMENTS_PER_CHUNK
      .number_measurements = MEASUREMENTS_PER_CHUNK,
#else
      .number_measurements = 500,
#endif
  };
  dudect_ctx_t ctx;

  dudect_init(&ctx, &config);

  /*
    持續呼叫 dudect_main() 直到
     - 回傳非 DUDECT_NO_LEAKAGE_EVIDENCE_YET，或
     - 測試時間超過預期（建議搭配 timeout 使用）
    例如執行 20 分鐘：
      $ timeout 1200 ./your-executable
  */
  dudect_state_t state = DUDECT_NO_LEAKAGE_EVIDENCE_YET;
  while (state == DUDECT_NO_LEAKAGE_EVIDENCE_YET) {
    state = dudect_main(&ctx);
  }
  dudect_free(&ctx);
  return (int)state;
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  run_test();
  return 0;
}

