#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OSU_SONGS_DIR "C:/Users/gabri/AppData/Local/osu!/Songs/"
#define OSU_MAP_DIR "677573 AAAA - Hoshi o Kakeru Adventure _ we are forever friends! _ [Long ver]"
#define OSU_MAP_FILE_DIR "AAAA - Hoshi o Kakeru Adventure ~ we are forever friends! ~ [Long ver.] (Battle) [Imagined Voyage].osu"
#define TIMING_POINTS_LINE "[TimingPoints]"
#define HIT_OBJECTS_LINE "[HitObjects]"
#define BUFFER_LENGTH 255

float getBPM(char buffer[BUFFER_LENGTH]);

void calcula_streams_osu_beatmap(FILE *fp, float bpm);

void calcula_um_tres_osu_beatmap(FILE *fp, float bpm);

int get_offset(char buffer[BUFFER_LENGTH]);

int get_tipo(char buffer[BUFFER_LENGTH]);


int main(void) {
    FILE *fp;
    int line_count = 0;
    char buffer[BUFFER_LENGTH];
    char subTiming[BUFFER_LENGTH];
    char subHitObj[BUFFER_LENGTH];
    float bpm = 0;

    char osu_dir[255];
    strcpy(osu_dir, OSU_SONGS_DIR);
    strcat(osu_dir, OSU_MAP_DIR);
    strcat(osu_dir, "/");
    strcat(osu_dir, OSU_MAP_FILE_DIR);

    if ((fp = fopen(osu_dir, "r")) == NULL) {
        printf("Erro ao abrir arquivo.");
        exit(0);
    }

    while (fgets(buffer, BUFFER_LENGTH, fp)) {
        memcpy(subTiming, &buffer[0], 14);
        subTiming[14] = '\0';

        memcpy(subHitObj, &buffer[0], 14);
        subHitObj[12] = '\0';

        if (strcmp(subTiming, TIMING_POINTS_LINE) == 0) { // chegou na linha de timing points
            // printf("line[%d]: %s", line_count, buffer);
            fgets(buffer, BUFFER_LENGTH, fp); // le o primeiro timing point
            printf("BPM: %.5f\n", 60000 / getBPM(buffer));
            printf("OFFSET DIFF (1/1): %.5f\n", getBPM(buffer));
            printf("OFFSET DIFF STREAM: %.5f\n", getBPM(buffer) / 4);
            bpm = getBPM(buffer);
            line_count++;
            // printf("line[%d]: %s", line_count, buffer);
            memcpy(subTiming, &buffer[0], 14);
            subTiming[14] = '\0';
        }

        if (strcmp(subHitObj, HIT_OBJECTS_LINE) == 0) { // chegou na linha de hit objects
            // printf("line[%d]: %s", line_count, buffer);
            /*fgets(buffer, BUFFER_LENGTH, fp); // le o primeiro hit object
            line_count++;
            printf("line[%d]: %s", line_count, buffer);
            memcpy(subHitObj, &buffer[0], 14);
            subHitObj[12] = '\0';*/

            calcula_streams_osu_beatmap(fp, bpm);
        }

        line_count++;
    }

    fclose(fp);
}

float getBPM(char buffer[BUFFER_LENGTH]) {
    float bpm;
    char buffer_bpm[255], ch;
    int cont = 0, size = 0;
    // printf("buffer: %s len: %llu\n", buffer, strlen(buffer));

    for (int i = 0; i < strlen(buffer); i++) {
        // printf("ch[%d]: %c\n", i, ch);
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

    // printf("buffer_bpm: %s\n", buffer_bpm);
    // printf("buffer_bpm (int): %.14f\n", atof(buffer_bpm));

    bpm = (float) atof(buffer_bpm);
    // printf("bpm: %.5f\n", bpm);

    return bpm;
}

int get_offset(char buffer[BUFFER_LENGTH]) {
    int offset;
    char buffer_offset[255], ch;
    int cont = 0, size = 0;
    // printf("buffer: %s len: %llu\n", buffer, strlen(buffer));

    for (int i = 0; i < strlen(buffer); i++) {
        // printf("ch[%d]: %c\n", i, ch);
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

    // printf("buffer_offset: %s\n", buffer_offset);
    // printf("buffer_offset (int): %.14f\n", atof(buffer_offset));

    offset = atoi(buffer_offset);
    // printf("bpm: %.5f\n", bpm);

    return offset;
}

void calcula_streams_osu_beatmap(FILE *fp, float bpm) {
    char buffer[BUFFER_LENGTH];
    int cont = 0, total = 0, tipo, offset_hitobject, offset_hitobject_antes, diff_objs, diff_stream_esperado =
            (int) bpm / 4;
    int vet[33];

    for (int i = 0; i < 33; ++i) {
        vet[i] = 0;
    }

    printf("diff_stream_esperado: %d\n", diff_stream_esperado);

    fgets(buffer, BUFFER_LENGTH, fp);
    offset_hitobject_antes = get_offset(buffer);

    while (fgets(buffer, BUFFER_LENGTH, fp)) {
        // printf("buffer: %s\n", buffer);
        offset_hitobject = get_offset(buffer);
        tipo = get_tipo(buffer);
        //printf("offset_antes: %d   ###   offset: %d   ###   diff: %d\n", offset_hitobject_antes, offset_hitobject, offset_hitobject - offset_hitobject_antes);

        diff_objs = offset_hitobject - offset_hitobject_antes;

        if (diff_objs + 2 >= diff_stream_esperado && diff_objs - 2 <= diff_stream_esperado && tipo != 12) { // é stream
            cont++;
        } else {
            if (cont >= 32)
                vet[32]++;
            else
                vet[cont]++;
            cont = 0;
        }
        offset_hitobject_antes = offset_hitobject;

    }

    printf("\n---------------------\nVetor: \n");

    for (int i = 0; i < 33; ++i) {
        if (vet[i] != 0)
            printf("%d: %d\n", i + 1, vet[i]);
    }

    for (int i = 0; i < 33; ++i) {
        total += vet[i];
    }

    printf("Total: %d", total);
}


void calcula_um_tres_osu_beatmap(FILE *fp, float bpm) {
    char buffer[BUFFER_LENGTH];
    int cont = 0, total = 0, tipo, offset_hitobject, offset_hitobject_antes, diff_objs, diff_stream_esperado =
            (int) bpm / 3;
    int vet[33];

    for (int i = 0; i < 33; ++i) {
        vet[i] = 0;
    }

    printf("diff_stream_esperado: %d\n", diff_stream_esperado);

    fgets(buffer, BUFFER_LENGTH, fp);
    offset_hitobject_antes = get_offset(buffer);

    while (fgets(buffer, BUFFER_LENGTH, fp)) {
        // printf("buffer: %s\n", buffer);
        offset_hitobject = get_offset(buffer);
        tipo = get_tipo(buffer);
        //printf("offset_antes: %d   ###   offset: %d   ###   diff: %d\n", offset_hitobject_antes, offset_hitobject, offset_hitobject - offset_hitobject_antes);

        diff_objs = offset_hitobject - offset_hitobject_antes;

        if (diff_objs + 2 >= diff_stream_esperado && diff_objs - 2 <= diff_stream_esperado && tipo != 12) { // é stream
            cont++;
        } else {
            if (cont >= 32)
                vet[32]++;
            else
                vet[cont]++;
            cont = 0;
        }
        offset_hitobject_antes = offset_hitobject;

    }

    printf("\n---------------------\nVetor: \n");

    for (int i = 0; i < 33; ++i) {
        if (vet[i] != 0)
            printf("%d: %d\n", i + 1, vet[i]);
    }

    for (int i = 0; i < 33; ++i) {
        total += vet[i];
    }

    printf("Total: %d", total);
}


int get_tipo(char buffer[BUFFER_LENGTH]) {
    int tipo, cont = 0, size = 0;
    char ch, buffer_tipo[5];

    for (int i = 0; i < strlen(buffer); i++) {
        // printf("ch[%d]: %c\n", i, ch);
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