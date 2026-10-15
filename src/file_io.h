#ifndef FILE_IO_H
#define FILE_IO_H
#include <stdint.h>
#include <stddef.h>

// Make this into a circular doubly linked list
typedef struct File_Explorer_Row {
    char *file_name;
    uint16_t prev_idx, next_idx;
    uint8_t is_dir;
} File_Explorer_Row;

typedef struct {
    File_Explorer_Row *rows;
    uint16_t used;
    uint16_t unused;
    uint16_t num_rows;
    uint16_t max_rows;
    uint16_t max_str_len;
    uint16_t selected_row;
} File_Explorer;

void file_explorer_init(File_Explorer *fe, uint16_t max_rows, uint16_t max_str_len);
void file_explorer_free(File_Explorer *fe);

uint8_t file_explorer_save(File_Explorer *fe, const char *file_name, size_t strlen, uint8_t *data, int w, int h, int num_chnl);
uint8_t file_explorer_load(File_Explorer *fe, const char *file_name, size_t strlen, uint8_t *data, int *w, int *h, int *num_chnl);
uint8_t file_explorer_remove(File_Explorer *fe, const char *file_name);

#endif // FILE_IO_H
