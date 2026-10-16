#include "file_io.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

// TODO: THIS CODE NEEDS TO BE MODIFIED TO ALSO WORK ON WINDOWS. DO IN HOLIDAYS

// See these libraries for how to do Windows compatibility:
// https://github.com/cxong/tinydir/tree/master
// https://github.com/tronkko/dirent
// TODO: FILTER OUT ANYTHING THAT ISN'T JPEG/PNG/BMP or a folder
void file_explorer_init(File_Explorer *fe, uint16_t max_rows, uint16_t max_str_len) {
    fe->num_rows = 0;
    fe->max_rows = max_rows;
    fe->max_str_len = max_str_len;
    fe->rows = malloc(sizeof(File_Explorer_Row) * max_rows);

    // Directory opening and parsing code modified from here:
    // https://stackoverflow.com/questions/12489/how-do-you-get-a-directory-listing-in-c
    // https://stackoverflow.com/questions/4553012/checking-if-a-file-is-a-directory-or-just-a-file
    DIR *dp;
    struct dirent *ep;
    dp = opendir("./");
    if(dp != NULL) { // Open the cwd
        struct stat path_stat;

        // Read all of the files stored in the cwd
        while((ep = readdir(dp)) != NULL) {
            if(fe->num_rows >= fe->max_rows) break;        // TODO: FOR NOW, NEED TO FIX LATER

            // Copy the file name
            fe->rows[fe->num_rows].file_name = strdup(ep->d_name);
            // Check if the file is a directory using stat. NOTE: NOT CROSS-PLATFORM
            stat(ep->d_name, &path_stat);

            fe->rows[fe->num_rows].is_dir = S_ISDIR(path_stat.st_mode);
            fe->rows[fe->num_rows].next_idx = (fe->num_rows + 1) % fe->max_rows;
            fe->rows[fe->num_rows].prev_idx = (fe->num_rows > 0) ? fe->num_rows - 1 : 0;
            fe->num_rows++; // Move to the next row
        }

        if(fe->num_rows > 0) {
            fe->rows[0].prev_idx = fe->num_rows - 1;
            fe->used = 0;
        }
        if(fe->num_rows < fe->max_rows) {
            fe->unused = fe->num_rows;
            for(int i = fe->num_rows; i < fe->max_rows; i++) {
                fe->rows[i].prev_idx = (i == fe->num_rows) ? fe->max_rows - 1 : i - 1;
                fe->rows[i].next_idx = (i == fe->max_rows - 1) ? fe->num_rows : i + 1;
            }
        }

        (void) closedir(dp); // Free the resource
    }
    else {
        perror("Couldn't open the current working directory\n");
    }
}

void file_explorer_free(File_Explorer *fe) {
    for(size_t i = 0; i < fe->num_rows; i++) {
        free(fe->rows[i].file_name);
    }
    free(fe->rows);
}

// TODO: This function adds a new row even if the file is not being saved to cwd. Fix at a later date
uint8_t file_explorer_save(File_Explorer *fe, const char *file_name, size_t strlen, uint8_t *data, int w, int h, int num_chnl) {
    if(strlen < 6) return 1; //Early exit if the filename is too short (max 4 chars for filetype, 1 for '.' and then at least one for the name)

    uint16_t curr = fe->used;
    uint8_t found = 0;
    while(!found) {
        if(!strcmp(fe->rows[curr].file_name, file_name)) {
            found = 1;
            if(fe->rows[curr].is_dir) fe->rows[curr].is_dir = 0;  // If a directory is the same name as the file being saved, reset it to a file
            break;
        }
        if(fe->rows[curr].next_idx == fe->used) break;
        curr = fe->rows[curr].next_idx;
    }
    //If no row exists in the explorer with this filename, need to make a new one
    if(!found) {
        // TODO: Use realloc instead of an error message
        if(fe->num_rows == fe->max_rows) fprintf(stderr, "Error: Trying to insert row into full file explorer. This should not happen\n");
        File_Explorer_Row inserted = fe->rows[fe->unused];

        if(fe->num_rows + 1 == fe->max_rows) fe->unused = UINT16_MAX;
        else {
            fe->rows[inserted.prev_idx].next_idx = inserted.next_idx;
            fe->rows[inserted.next_idx].prev_idx = inserted.prev_idx;
        }

        if(fe->used == UINT16_MAX) fe->used = curr;
        else {
            inserted.prev_idx = fe->rows[fe->used].prev_idx;
            inserted.next_idx = fe->used;
            fe->rows[fe->rows[fe->used].prev_idx].next_idx = curr;
            fe->rows[fe->used].prev_idx = curr;
        }
        fe->num_rows++;
    }

    char buf[8];
    int len = sizeof(buf) - 1;
    for(int i = 0; i < len; i++) {
        buf[len - i] = file_name[strlen - i - 1];
    }
    buf[len] = '\0'; // Null terminate the string for easy use

    if(!strcmp(buf, ".png")) {
        if(!stbi_write_png(file_name, w, h, num_chnl, data, w * num_chnl)) {
            printf("Error: Failed to write %s\n", file_name);
            return 1;
        }
    }
    else if(!strcmp(buf, ".jpg") || !strcmp(buf, ".jpeg")) {
        if(!stbi_write_jpg(file_name, w, h, num_chnl, data, 70)) {
            printf("Error: Failed to write %s\n", file_name);
            return 1;
        }
    }
    else if(!strcmp(buf, ".bmp")) {
        if(!stbi_write_bmp(file_name, w, h, num_chnl, data)) {
            printf("Error: Failed to write %s\n", file_name);
            return 1;
        }
    }
    else {
        fprintf(stderr, "Error: Could not save file. Invalid filetype\n");
        return 1;
    }


    return 0;
}

uint8_t file_explorer_load(File_Explorer *fe, const char *file_name, size_t strlen, uint8_t *data, int *w, int *h, int *num_chnl) {
    if(strlen < 6) return 1; //Early exit if the filename is too short (max 4 chars for filetype, 1 for '.' and then at least one for the name)

    char buf[8];
    int len = sizeof(buf) - 1;
    for(int i = 0; i < len; i++) {
        buf[len - i] = file_name[strlen - i - 1];
    }
    buf[len] = '\0'; // Null terminate the string for easy use
    if(strcmp(buf, ".png") && strcmp(buf, "jpg") && strcmp(buf, "jpeg") && strcmp(buf, "bmp"))
        printf("Error: Could not load file. Invalid filetype\n");


    data = stbi_load(file_name, w, h, num_chnl, 0);
    if (!data) {
        fprintf(stderr, "Failed to load texture at path %s\n", file_name);
        return 1;
    }
    return 0;
}
uint8_t file_explorer_remove(File_Explorer *fe, const char *file_name) {
    uint16_t curr = fe->used;
    uint8_t found = 0;
    while(!found) {
        if(!strcmp(fe->rows[curr].file_name, file_name)) {
            found = 1;
            break;
        }
        if(fe->rows[curr].next_idx == fe->used) break;
        curr = fe->rows[curr].next_idx;
    }

    if(!found) {
        fprintf(stderr, "Error: Failed to find file\n");
        return 1;
    }
    else if(!remove(file_name)){
        fprintf(stderr, "Error: Failed to delete file\n");
        return 1;
    }
    else {
        if(fe->num_rows == 0) fprintf(stderr, "Error: Trying to remove a row from an empty file explorer. This should not happen\n");
        File_Explorer_Row removed = fe->rows[curr];
        if(fe->num_rows == 1) fe->used = UINT16_MAX;
        else {
            fe->rows[removed.prev_idx].next_idx = removed.next_idx;
            fe->rows[removed.next_idx].prev_idx = removed.prev_idx;
        }

        if(fe->unused == UINT16_MAX) fe->unused = curr;
        else {
            removed.prev_idx = fe->rows[fe->unused].prev_idx;
            removed.next_idx = fe->unused;
            fe->rows[fe->rows[fe->unused].prev_idx].next_idx = curr;
            fe->rows[fe->unused].prev_idx = curr;
        }
        fe->num_rows--;
        return 0;
    }
}
