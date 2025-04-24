#include <cstddef>
#include <sys/types.h>

#define OPEN_MODES O_RDONLY
#define WRITE_FLAGS (O_RDWR | O_CREAT | O_APPEND)
#define WRITE_MODE 0644

#define PATH "/home/CHOCEK/tmp/megatron/db/"
#define SCHEMA "esquema.txt"

constexpr off_t END = -1;
constexpr size_t BUFFER_SIZE = 1024;
constexpr auto TMP_SCHEMAS = "schema_tmp.txt";
constexpr auto TMP_RESULT = ".tmp.txt";

