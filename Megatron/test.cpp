#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cassert>
#include <errno.h>

constexpr size_t BLOCK_SIZE = 512; // tamaño mínimo típico para O_DIRECT
constexpr const char* DB_FILE = "database.db";

// Función para alocar memoria alineada
void* aligned_alloc_block() {
    void* ptr = nullptr;
    if (posix_memalign(&ptr, BLOCK_SIZE, BLOCK_SIZE) != 0) {
        perror("posix_memalign");
        exit(1);
    }
    return ptr;
}

// Inicializa archivo con tamaño suficiente
void init_file(const char* path) {
    int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        exit(1);
    }
    // Reservamos 10 bloques
    if (ftruncate(fd, 10 * BLOCK_SIZE) < 0) {
        perror("ftruncate");
        exit(1);
    }
    close(fd);
}

// Escribe un bloque de datos alineado en offset
void write_record(const char* path, size_t offset_block, const char* data) {
    int fd = open(path, O_WRONLY | O_DIRECT);
    if (fd < 0) {
        perror("open write O_DIRECT");
        exit(1);
    }

    void* buffer = aligned_alloc_block();
    memset(buffer, 0, BLOCK_SIZE);
    strncpy((char*)buffer, data, BLOCK_SIZE - 1); // copiamos el string

    off_t offset = offset_block * BLOCK_SIZE;
    ssize_t written = pwrite(fd, buffer, BLOCK_SIZE, offset);
    if (written != BLOCK_SIZE) {
        perror("pwrite");
        std::cerr << "Escrito: " << written << std::endl;
    } else {
        std::cout << "[OK] Escribí en bloque " << offset_block << std::endl;
    }

    free(buffer);
    close(fd);
}

// Lee un bloque y lo imprime
void read_record(const char* path, size_t offset_block) {
    int fd = open(path, O_RDONLY | O_DIRECT);
    if (fd < 0) {
        perror("open read O_DIRECT");
        exit(1);
    }

    void* buffer = aligned_alloc_block();
    off_t offset = offset_block * BLOCK_SIZE;

    ssize_t read_bytes = pread(fd, buffer, BLOCK_SIZE, offset);
    if (read_bytes < 0) {
        perror("pread");
    } else {
        std::cout << "[OK] Leído de bloque " << offset_block << ":\n";
        std::cout << std::string((char*)buffer, 64) << "...\n";
    }

    free(buffer);
    close(fd);
}

// MAIN con múltiples pruebas
int main() {
    std::cout << "Inicializando archivo de base de datos..." << std::endl;
    init_file(DB_FILE);

    std::cout << "\nEscribiendo registros..." << std::endl;
    write_record(DB_FILE, 0, "Usuario: Alice, ID: 1001");
    write_record(DB_FILE, 1, "Usuario: Bob, ID: 1002");

    std::cout << "\nLeyendo registros..." << std::endl;
    read_record(DB_FILE, 0);
    read_record(DB_FILE, 1);

    return 0;
}
