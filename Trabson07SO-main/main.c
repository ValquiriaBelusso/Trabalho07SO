#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// #pragma pack(1)
typedef struct fat_BS {
    unsigned char bootjmp[3];
    unsigned char oem_name[8];
    unsigned short bytes_per_sector;
    unsigned char sectors_per_cluster;
    unsigned short reserved_sector_count;
    unsigned char table_count;
    unsigned short root_entry_count;
    unsigned short total_sectors_16;
    unsigned char media_type;
    unsigned short table_size_16;
    unsigned short sectors_per_track;
    unsigned short head_side_count;
    unsigned int hidden_sector_count;
    unsigned int total_sectors_32;
    // this will be cast to it's specific type once the driver> unsigned char extended_section[54];
    unsigned char extended_section[54];
} __attribute__((packed)) fat_BS_t;

// #pragma pack(1)
typedef struct eight_three {
    unsigned char name[8];
    unsigned char ext[3];
    unsigned char file_type;
    int size;
    int first_cluster;
} __attribute__((packed)) eight_three_t;

void printEightThree(eight_three_t entry) {
    for (int i = 0; i < 8; i++) {
        if (entry.name[i] == ' ') break;

        printf("%c", entry.name[i]);
    }

    if (entry.file_type == 0x20) printf(".");

    for (int i = 0; i < 3; i++) {
        if (entry.ext[i] == ' ') break;
        printf("%c", entry.ext[i]);
    }

    printf(" \t Size: %d \tbytes\n", entry.size);
}

int match(eight_three_t entry, char *obj) {
    char fname[12];
    int len = 0;
    for (int i = 0; i < 8; i++) {
        if (entry.name[i] == ' ') break;
        fname[i] = entry.name[i];
        len++;
    }

    if (entry.file_type == 0x20) {
        fname[len] = '.';
        len++;
    }

    for (int i = 0; i < 3; i++) {
        if (entry.ext[i] == ' ') break;
        fname[len] = entry.ext[i];
        len++;
    }

    for (int i = 0; i < len; i++) {
        if (fname[i] != obj[i]) return 0;
    }

    return 1;
}

int main()
{

    FILE *fp;

    char filename[100];

    printf("Enter filename (.img): ");
    scanf("%s", filename);

    fp = fopen(filename, "rb");

    if (fp == NULL) {
        printf("Couldn't open file");
        return 0;
    }

    fat_BS_t boot_record;
    fseek(fp, 0, SEEK_SET);
    fread(&boot_record, sizeof(fat_BS_t), 1, fp);

    int root_dir_sectors = boot_record.root_entry_count * 32 / boot_record.bytes_per_sector;

    unsigned int fat1 = boot_record.reserved_sector_count;
    unsigned int fat2 = fat1 + boot_record.table_size_16;
    unsigned int root = boot_record.reserved_sector_count + (boot_record.table_count * boot_record.table_size_16);
    unsigned int data = root + (boot_record.root_entry_count * 32 / boot_record.bytes_per_sector);

    printf("FILE: %s\n", filename);
    printf("BOOT RECORD INFO:\n");
    printf("Bytes per Sector ----------------------- %4hd\n", boot_record.bytes_per_sector);
    printf("Sector per cluster --------------------- %4x\n", boot_record.sectors_per_cluster);
    printf("Number of reserved sectors ------------- %4hd\n", boot_record.reserved_sector_count);
    printf("Number of FATs ------------------------- %4x\n", boot_record.table_count);
    printf("Sectors per FAT ------------------------ %4hd\n", boot_record.table_size_16);
    printf("Number of entries in root directory ---- %4hd\n", boot_record.root_entry_count);
    printf("Sectors of root directory -------------- %4hd\n", root_dir_sectors);

    printf("Localization of:\n");
    printf("FAT 1          at sector %6d, byte %8d  (0x%X)\n", fat1, fat1 * boot_record.bytes_per_sector, fat1 * boot_record.bytes_per_sector);
    printf("FAT 2          at sector %6d, byte %8d  (0x%X)\n", fat2, fat2 * boot_record.bytes_per_sector, fat2 * boot_record.bytes_per_sector);
    printf("ROOT DIRECTORY at sector %6d, byte %8d  (0x%X)\n", root, root * boot_record.bytes_per_sector, root * boot_record.bytes_per_sector);
    printf("DATA SECTION   at sector %6d, byte %8d  (0x%X)\n", data, data * boot_record.bytes_per_sector, data * boot_record.bytes_per_sector);
    printf("----------------------------------------------------------------------\n");

    unsigned short *FAT;
    FAT = malloc(boot_record.table_size_16 * boot_record.bytes_per_sector);
    fseek(fp, boot_record.reserved_sector_count * boot_record.bytes_per_sector, SEEK_SET);
    fread(FAT, boot_record.table_size_16 * boot_record.bytes_per_sector, 1, fp); 

    unsigned char *ROOT_DIRECTORY;
    ROOT_DIRECTORY = malloc(boot_record.root_entry_count * 32);
    fseek(fp, boot_record.reserved_sector_count * boot_record.bytes_per_sector + (boot_record.table_count * boot_record.table_size_16 * boot_record.bytes_per_sector), SEEK_SET);
    fread(ROOT_DIRECTORY, root_dir_sectors * 32, 1, fp); 

    eight_three_t *files = malloc(sizeof(eight_three_t) * root_dir_sectors);
    unsigned char current_entry[32];
    int entry_count = 0, pos = 0;
    eight_three_t file;
    int size, cluster;

    while (entry_count < boot_record.root_entry_count) {
        // 0x00 - Empty
        if (ROOT_DIRECTORY[entry_count * 32] == 0x00) break;

        // 0xE5 - File removed
        if (ROOT_DIRECTORY[entry_count * 32] == 0xE5) {
            entry_count++;
            continue;
        }

        // 0x0F - LNF
        if (ROOT_DIRECTORY[(entry_count * 32) + 11] == 0x0F) {
            entry_count++;
            continue;
        }

        for (int j = 0; j < 32; j++) {
            current_entry[j] = ROOT_DIRECTORY[j + (entry_count * 32)];
        }

        // Ox10 - Directory || 0x20 - Archive 
        if (current_entry[11] == 0x10 || current_entry[11] == 0x20) {
            size = current_entry[31] << 24 | current_entry[30] << 16 | current_entry[29] << 8 | current_entry[28];
            cluster = current_entry[27] << 8 | current_entry[26];
        }

        for (int k = 0; k < 8; k++) {
            file.name[k] = current_entry[k];
        }

        for (int k = 0; k < 3; k++) {
            file.ext[k] = current_entry[k + 8];
        }
        file.file_type = current_entry[11];
        file.size = size;
        file.first_cluster = cluster;

        files[pos] = file;
        pos++;
        entry_count++;
    }

    for (int k = 0; k < pos; k++) {
        printf("[%d] ", k);
        printEightThree(files[k]);
    }

    int found = 0;
    while (found == 0) {
        printf("Open > ");
        char obj[12];
        scanf("%s", obj);
        for (int k = 0; k < pos; k++) {
            if (match(files[k], obj)) {
                found = 1;
                pos = k;
            }
        }
    }
    printf("Found: ");
    file = files[pos];
    printEightThree(file);

    int n = (file.size / boot_record.bytes_per_sector) + 1;
    int file_clusters[n];
    int clusters = 1;
    file_clusters[0] = file.first_cluster;

    for (int i = 0; i < n; i++) {
        file_clusters[i + 1] = FAT[file_clusters[i]];

        if (file_clusters[i + 1] >= 0xFFFF) break;
        
        clusters++;
    }

    int bytes_per_cluster = boot_record.sectors_per_cluster * boot_record.bytes_per_sector;
    if (file.file_type == 0x10) {
        unsigned char DATA[bytes_per_cluster];

        fseek(fp, (data + ((file_clusters[0] - 2) * boot_record.sectors_per_cluster)) * boot_record.bytes_per_sector, SEEK_SET);

        fread(&DATA, bytes_per_cluster, 1, fp);
        eight_three_t dir_file;
        entry_count = 0;
        while (entry_count < bytes_per_cluster / 32) {
            // 0x00 - Empty
            if (DATA[entry_count * 32] == 0x00) 
                break;

            // 0xE5 - File removed
            if (DATA[entry_count * 32] == 0xE5) {
                entry_count++;
                continue;
            }

            // 0x0F - LNF
            if (DATA[(entry_count * 32) + 11] == 0x0F) {
                entry_count++;
                continue;
            }

            for (int j = 0; j < 32; j++) {
                current_entry[j] = DATA[j + (entry_count * 32)];
            }

            if (current_entry[11] == 0x10 || current_entry[11] == 0x20) {
                size = current_entry[31] << 24 | current_entry[30] << 16 | current_entry[29] << 8 | current_entry[28];
                cluster = current_entry[27] << 8 | current_entry[26];
            }

            for (int k = 0; k < 8; k++) {
                dir_file.name[k] = current_entry[k];
            }

            for (int k = 0; k < 3; k++) {
                dir_file.ext[k] = current_entry[k + 8];
            }
            dir_file.file_type = current_entry[11];
            dir_file.size = size;
            dir_file.first_cluster = cluster;
            printEightThree(dir_file);
            entry_count++;
        }
    }

    else if (file.file_type == 0x20)
    {
        unsigned char DATA[file.size];
        for (int i = 0; i < clusters; i++)
        {
            fseek(fp, (data + ((file_clusters[i] - 2) * boot_record.sectors_per_cluster)) * boot_record.bytes_per_sector, SEEK_SET);
            int num_bytes = i * bytes_per_cluster;
            if (i == clusters - 1)
            {
                // alloc only the rest of bytes - fragmentation
                fread(&DATA[num_bytes], file.size - (num_bytes), 1, fp);
            }
            else
                fread(&DATA[num_bytes], bytes_per_cluster, 1, fp);
        }

        printf("\n");
        for (int i = 0; i < file.size; i++)
        {
            printf("%c", DATA[i]);
        }
    }

    free(ROOT_DIRECTORY);
    free(FAT);
    fclose(fp);
}