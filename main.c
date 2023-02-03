#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define OSU_PATH "C:\\Users\\gabri\\AppData\\Local\\osu!\\Songs\\"
#define TIMING_POINTS_LINE "[TimingPoints]"
#define HIT_OBJECTS_LINE "[HitObjects]"
#define BUFFER_SIZE 255

int print_dot_osu(char path[512]);

void le_arquivo(char osu_dir[BUFFER_SIZE]);

float get_bpm(char buffer[BUFFER_SIZE]);

int soma_streams(FILE *fp, float bpm);

int get_offset(char buffer[BUFFER_SIZE]);

int get_tipo(char buffer[BUFFER_SIZE]);

int is_stream_map(int vet[300]);


int main() {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char path[512];

    strcpy(path, OSU_PATH);
    dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) {
        strcpy(path + strlen(path), entry->d_name);
        strcat(path, "\\");
        stat(path, &statbuf);
        if (S_ISDIR(statbuf.st_mode)) {
            print_dot_osu(path);
        }
        path[strlen(path) - strlen(entry->d_name) - 1] = '\0';
    }

    closedir(dir);
    return 0;
}

int print_dot_osu(char path[512]) {
    DIR *dir;
    struct dirent *entry;
    char path_copy[512];

    dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".osu") != NULL) {
            strcpy(path_copy, path);
            strcat(path_copy, entry->d_name);
            le_arquivo(path_copy);
        }
    }

    closedir(dir);
    return 0;
}

void le_arquivo(char osu_dir[BUFFER_SIZE]) {
    FILE *fp;
    char buffer[BUFFER_SIZE];
    char subTiming[BUFFER_SIZE];
    char subHitObj[BUFFER_SIZE];
    float bpm = 0;
    char map_name[BUFFER_SIZE];

    if ((fp = fopen(osu_dir, "r")) == NULL) {
        printf("Erro ao abrir arquivo %s", osu_dir);
        return;
    }

    while (fgets(buffer, BUFFER_SIZE, fp)) {
        memcpy(subTiming, &buffer[0], 14);
        subTiming[14] = '\0';

        memcpy(subHitObj, &buffer[0], 14);
        subHitObj[12] = '\0';

        if (strcmp(subTiming, TIMING_POINTS_LINE) == 0) {
            fgets(buffer, BUFFER_SIZE, fp);
            bpm = get_bpm(buffer);
            //printf("\n\nBPM: %.5f\n", 60000 / bpm);
        }

        if (strcmp(subHitObj, HIT_OBJECTS_LINE) == 0) {
            if (soma_streams(fp, bpm) == 1) {
                strcpy(map_name, &osu_dir[40]);
                printf("%s\n\n", map_name);
            }
        }
    }

    fclose(fp);
}

float get_bpm(char buffer[BUFFER_SIZE]) {
    float bpm;
    char buffer_bpm[255], ch;
    int cont = 0, size = 0;

    for (int i = 0; i < strlen(buffer); i++) {
        ch = buffer[i];

        if (ch == ',') {
            cont++;
        }

        if (cont == 1 && ch != ',') {
            buffer_bpm[size++] = ch;
        }

        if (cont == 2)
            break;
    }

    buffer_bpm[size] = '\0';

    bpm = (float) atof(buffer_bpm);

    return bpm;
}

int get_offset(char buffer[BUFFER_SIZE]) {
    int offset;
    char buffer_offset[255], ch;
    int cont = 0, size = 0;

    for (int i = 0; i < strlen(buffer); i++) {
        ch = buffer[i];

        if (ch == ',') {
            cont++;
        }

        if (cont == 2 && ch != ',') {
            buffer_offset[size++] = ch;
        }

        if (cont == 3)
            break;
    }

    buffer_offset[size] = '\0';

    offset = atoi(buffer_offset);

    return offset;
}

int get_tipo(char buffer[BUFFER_SIZE]) {
    int tipo, cont = 0, size = 0;
    char ch, buffer_tipo[5];

    for (int i = 0; i < strlen(buffer); i++) {
        ch = buffer[i];

        if (ch == ',') {
            cont++;
        }

        if (cont == 3 && ch != ',') {
            buffer_tipo[size++] = ch;
        }

        if (cont == 4)
            break;
    }

    buffer_tipo[size] = '\0';

    tipo = atoi(buffer_tipo);

    return tipo;
}

int soma_streams(FILE *fp, float bpm) {
    char buffer[BUFFER_SIZE];
    int cont = 0, tipo, tamanho_max_stream = 300;
    int offset_hit_object, offset_hit_object_antes, diferenca_hit_objects, diferenca_esperada = (int) (bpm / 4 + 0.5f);
    int vet[tamanho_max_stream];

    memset(vet, 0, sizeof(vet));

    if (fgets(buffer, BUFFER_SIZE, fp)) {
        offset_hit_object_antes = get_offset(buffer);
    } else {
        return 727;
    }

    while (fgets(buffer, BUFFER_SIZE, fp)) {
        offset_hit_object = get_offset(buffer);
        tipo = get_tipo(buffer);

        diferenca_hit_objects = offset_hit_object - offset_hit_object_antes;

        if (abs(diferenca_hit_objects - diferenca_esperada) <= 2 && tipo != 12) {
            cont++;
        } else {
            vet[cont >= tamanho_max_stream ? tamanho_max_stream - 1 : cont]++;
            cont = 0;
        }
        offset_hit_object_antes = offset_hit_object;
    }

    vet[cont >= tamanho_max_stream ? tamanho_max_stream - 1 : cont]++;

    if (is_stream_map(vet) == 1) {
        return 1;
    }

    return 0;
}

/*
 * condition to be a stream map: more than 15 bursts with 5 or more notes each
 */
int is_stream_map(int vet[300]) {
    int sum = 0;

    for (int i = 5; i < 300; i++) {
        sum += vet[i];
    }

    if (sum >= 15) {
        return 1;
    } else {
        return 0;
    }
}

/*
 * maps with doubles
int is_stream_map(int vet[300]) {
    if (vet[2] >= 30) {
        return 1;
    } else {
        return 0;
    }
}*/
