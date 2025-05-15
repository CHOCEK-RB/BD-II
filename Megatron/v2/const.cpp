#include <fcntl.h>
#include <unistd.h>
constexpr char PATH[] = "/home/CHOCEK/tmp/megatron/db/";
constexpr char SCHEMA[] = "esquema.txt";
constexpr char SAVE_FD_TMP[] = ".save_fd_tmp.txt";
constexpr char INPUT_TMP[] = ".input_tmp.txt";
constexpr char RELATIONS_TMP[] = ".relations_tmp.txt";
constexpr char ATTRIBUTES_TMP[] = ".attributes_tmp.txt";

constexpr char SELECT_TMP[] = ".select_tmp.txt";
constexpr char FROM_TMP[] = ".from_tmp.txt";

constexpr int NOT_OPEN = -1;
constexpr int READ_WRITE_TRUNC_FLAGS = (O_RDWR | O_CREAT | O_TRUNC);
constexpr int READ_WRITE_FLAGS = (O_RDWR | O_CREAT);
constexpr int BYTES_FOR_ATTRIBUTES = 15;
