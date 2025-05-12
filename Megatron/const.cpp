#include <fcntl.h>
#include <sys/types.h>

#include <cstddef>

constexpr int READ_FLAGS = O_RDONLY;
constexpr int WRITE_FLAGS = (O_WRONLY | O_CREAT | O_TRUNC);
constexpr int READ_WRITE_FLAGS = (O_RDWR | O_CREAT | O_APPEND);
constexpr int READ_WRITE_TRUNC_FLAGS = (O_RDWR | O_CREAT | O_TRUNC);
#define WRITE_MODE 0644

constexpr off_t END = -1;
constexpr size_t BUFFER_SIZE = 1024;
constexpr int NOT_OPEN = -1;

constexpr char PATH[] = "~/tmp/megatron/db/";
constexpr char SCHEMA[] = "esquema.txt";
constexpr char TMP_SCHEMAS[] = ".schema_tmp.txt";
constexpr char TMP_ATTRIBUTES[] = ".attributes_tmp.txt";
constexpr char TMP_LINES[] = ".lines_tmp.txt";
constexpr char TMP_RESULT[] = ".tmp.txt";
constexpr char TMP_SIZE_ATTRIBUTES[] = ".size_attributes_tmp.txt";
constexpr char TMP_CONDITIONAL_ATTRIBUTES[] = ".conditional_attributes_tmp.txt";
constexpr char TMP_CONDITIONS[] = ".conditions_tmp.txt";
constexpr char TMP_COPY_CONDITIONS[] = ".copy_conditions_tmp.txt";

constexpr char ERR_OPEN_FILE[] = "No se pudo abrir el archivo : ";
constexpr char ERR_GETLINE_NOT_OPEN_FILE[] =
    "No se pudo obtener la linea, no se encuentra abierto el archivo : ";
constexpr char ERR_GETLINE_NOT_READ[] = "Error al leer el archivo : ";
constexpr char ERR_NOT_OPEN[] = "Error no se encuentra abierto el archivo : ";
constexpr char ERR_NOT_WRIT_NEW_LINE[] =
    "Error al trata de escribir una nueva linea en el archivo : ";
constexpr char ERR_WHERE[] =
    "ERROR: Revice que las condiciones esten bien escritas\n";

constexpr char ERR_NPOS_SCHEMA_ATTR[] =
    "ERROR: No se a podido encontrar la tablal del atributo : ";
constexpr char ERR_NPOS_ATTR[] =
    "ERROR: No se a podido encontrar el atributo : ";
constexpr char ERR_REPEAT_ATTR[] =
    "ERROR: Se repite en mas de 1 tabla el atributo : ";

constexpr char ERR_NOT_SCHEMA[] =
    "ERROR: No se a encontrador el esquema de la tabla : ";
