//
//  project_device_argument.c
//
//  Created by Shilin Chen on 8/21/17.
//  Copyright © 2017 Shilin Chen. All rights reserved

/* ------------ Project Device List Program Generation Utilty -----------*/

/* Instruction
 * First generate executable profile: gcc project_device.c -o project_list
 * After gernerate the executable file, type ./project_list fulldevicelist netlist outputfile (optional)
 * If you don't type in the ouptput file direction,
 * the program will automatically generate a file called device_list.txt in current directory
 * If you don't type in any arguments at the begining (type ./project_list)
 * Just follow the instruction to enter the full device list path, netlist file and output file
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void remove_file(char* path, char* file); 
void path2file(char* path);
void get_list(FILE* ff, const char* devicelist);
int read_file(FILE* fd, FILE* fn, FILE* fp);
void get_desc(FILE* fb, FILE* fp, char* device);
void display(char* file, int cols);
int compare(char* str1, char* str2);
int is_file(char* path);
// important characters
const char EOS = '\0';
const char SPACE = ' ';
const char RETURN = '\n';

// temprory file name
const char *FULL = "fulldevicelist.temp";
const char *DEVICE = "devicelist.temp";
const char *DB = "VIS_P25_DEV.txt";  // change this directory for specific libraray
static int max_length = 0;
int main(int argc, char* argv[])
{
    FILE *ff,*fn,*fp, *fd;
    char path[200];
    char file[200];
    int command = 0;
    char input[20];
    // get input:
    printf(" --------------- Project Device Utitly ----------------\n");
    printf("| 1) Generate Full Device List                         |\n");
    printf("| 2) Generate Project Device List                      |\n");
    printf(" ------------------------------------------------------\n");
    printf("| 3) Clean the Locks                                   |\n");
    printf("| 4) Clean the temp file                               |\n");
    printf(" ------------------------------------------------------\n");
    printf("Enter the utility number: ");
    scanf("%d", &command);
    printf("=======================================================\n");
    switch (command)
    {
        case 1:
            printf("Generate Full Device List\n");
            printf("Please input full device list path: ");
            scanf("%s", path);
            ff = fopen(path, "r");
            // check if successfully open the file
            if (ff == NULL)
            {
                perror("The Full Device List File Does Not Exist\n");
                return -1;
            }
            // check the full device list is file or directory
            if (is_file(path) == 0)
            {
                printf("Input is path.\n");
                fclose(ff);
                path2file(path);
                ff = fopen(FULL, "r");
            }
            else
            {
                printf("Input is file.\n");
            }
            printf("Output file name: ");
            scanf("%s", file);
            get_list(ff, file);
            fclose(ff);
            printf("Do you want to display the list?(y/n): ");
            scanf("%s",input);
            if (input[0] == 'y')
            {
                display(file, 1);
            }
            break;
        case 2:
            printf("Generate Project Device List\n");
            printf("Please input full device list path: ");
            scanf("%s", path);
            // check if successfully open the file
            ff = fopen(path, "r");
            if (ff == NULL)
            {
                perror("The Full Device List File Does Not Exist\n");
                return -1;
            }
                    // check the full device list is file or directory
            if (is_file(path) == 0)
            {
                printf("Input is path.\n");
                fclose(ff);
                path2file(path);
                ff = fopen(FULL, "r");
            }
            else
            {
                printf("Input is file.\n");
            }
            get_list(ff, DEVICE);
            fd = fopen(DEVICE, "r");
            fclose(ff);
            printf("Please input netlist file path: ");
            scanf("%s", file);
            fn = fopen(file, "r");
            printf("Please input project list file path: ");
            scanf("%s", file);
            fp = fopen(file, "w");
            if (fn == NULL)
            {
                perror("The Netlist List File Does Not Exist");
                return -1;
            }
            // check if project list file opened successfully
            if (fp == NULL)
            {
                perror("The Output Path Does Not Exist");
                return -1;
            }
            if (read_file(fd, fn, fp)== -1) return -1;	// get device list
            // close the opened file
            fclose(fd);
            fclose(fn);
            fclose(fp);
            printf("Do you want to display the list?(y/n): ");
            scanf("%s",input);
            if (input[0] == 'y')
            {
                display(file, 3);
            }
            break;
        case 3:
            printf("Clean all locks(*.cdslck).\n");
            printf("Please enter the library full path: ");
            scanf("%s", path);
            remove_file(path, "*.cdslck");
            break;
        case 4:
            printf("Clean all temp file.\n");
            printf("Please enter the full path: ");
            scanf("%s", path);
            break;
        default:
            printf("Clean all temp file");
            printf("Wrong command!\n");
            return -1;
        }
        
    
    return 0;
}

// generate the real device list without any redundant text.
void get_list(FILE* ff, const char* devicelist)
{
    FILE* fd = fopen(devicelist, "w");
    char path[200];      // device path
    char buffer[1000];   // string buffer
    char device[100];
    int i = 0;
    int count = 0;
    int end = 0;
    int len = 0;
    int flag = 0;
    int label = 1;
    while (fgets(buffer, sizeof(buffer), ff) != NULL)
    {
        count = 0;
        // check if the input string is too long
        if (flag == 0 && strlen(buffer) > sizeof(path))
        {
            printf("The path is too long, the result may be incorrect.\n");
            flag = 1;
        }
        strncpy(path, buffer, sizeof(path));
        // get the device name from the full path.
        for (i = strlen(path) - 1; i >= 0 && count < 2; i--) {
            if (path[i] == '/') {
                count++;
                if (count == 1)
                {
                    end = i;
                } else
                {
                    len = end - i - 1;
                    if (len > sizeof(device))
                    {
                        printf("The device name is too long, the result may be incorrect.\n");
                    }
                    strncpy(device, path + i + 1, end - i - 1);
                    device[end - i - 1] = EOS;
                }
            }
            
        }
        if (strlen(device) > max_length) max_length = strlen(device);
        fprintf(fd, "%s\n", device);
        
    }
    fclose(fd);
}

// seraching the netlist file, generate project device list.
int read_file(FILE* fd, FILE* fn, FILE* fp)
{
    long f_size;
    char line[300];   // string buffer
    char device[100]; // device name
    char word[100];   // netlist words
    char c;
    int i = 0;
    fseek(fn, 0, SEEK_END);
    f_size = ftell(fn);
    printf("Generating list...\n");
    printf("-------------------------------------------------------\n");
    if (f_size > 5000)
        printf ("The file is too big, please wait for couple of minutes.\n");
    int device_exist = 0; // device exist flag
    FILE *fb = fopen(DB, "r");
    if (fb == NULL) 
    {
        perror("can't open description file.");
        return -1;
    }
    if (fgets(line, sizeof(line), fb) != NULL) fprintf(fp, "%s", line);
    while (fgets(device, sizeof(device), fd) != NULL)
    {
        device_exist = 0;
        fseek(fn, 0, SEEK_SET);
        // find each word in the netlist file
        do
        {
            c = fgetc(fn);
            if (c == '*' || c == '=')
            {
                while (c != EOF && c != RETURN)
                {
                    c = fgetc(fn);
                }
                i = 0;
                continue;
            }
            if (c == EOS || c == SPACE || c == RETURN || c == EOF)
            {
                if (i > 0)
                {
                    strncpy(word, line, i);
                    word[i] = EOS;
                    // compare the word and device name
                    if (compare(device, word) == 0)
                    {
                        device_exist = 1;
                        break;
                    }
                    i = 0;
                }
            }
            else
            {
                line[i++] = c;
            }
        } while (c != EOF);
        if (device_exist == 1)
        {
            fseek(fb, 0, SEEK_SET);
            fgets(line, sizeof(line), fb);
            get_desc(fb, fp, device);
        }
    }
    fclose(fb);
    printf("list genreated!\n");
    return 0;
}

// compare the device name and word in netlist
// capital insensitive
int compare(char* str1, char* str2)
{
    int i = 0;
    while (1)
    {
        char c1 = str1[i];
        char c2 = str2[i];
        // reach end at the same time;
        if ((c1 == EOS || c1 == RETURN) && (c2 == EOS || c2 == RETURN))
        {
            str1[i] = EOS;
            return 0;
        }
        // the length of two strings are different
        if (c1 == EOS || c1 == RETURN || c2 == EOS || c2 == RETURN) return 1;
        // check letters with capital insensitive.
        if ((c1 <= 'Z' && c1 >= 'A') || (c1 >= 'a' && c1 <= 'z'))
        {
            int diff = c1 - c2;
            if (diff != 0 && diff != 'a' - 'A' && diff != 'A' - 'a')
            {
                return 1;
            }
        }
        // check other characters.
        else if (c1 != c2) return 1;
        i++;
    }
}

// get device description from datebase
void get_desc(FILE* fb, FILE* fp, char *device)
{
    static char line[300];
    char name[100];
    int i = 0;
    while (fgets(line, sizeof(line), fb) != NULL)
    {
        if (strlen(line) > max_length) max_length = strlen(line);
        i = 0;
        while (line[i] != EOS && line[i] != RETURN && line[i] != ' ') i++;
        strncpy(name, line, i);
        name[i] = EOS;
        if (compare(name, device) == 0)
        {
            fprintf(fp, "%s", line);
            return;
        }
    }
    fprintf(fp, "%s: description not found\n", device);
}


// convert device path to raw fulldevicelist
void path2file(char* path)
{
    char cmd_ls[500];
    strcpy(cmd_ls, "ls -d ");
    int len = strlen(path);
    if (path[len - 1] == '/')
    {
        path[len - 1] = EOS;
    }
    strcat(cmd_ls, path);
    strcat(cmd_ls, "/*/ > ");
    strcat(cmd_ls, FULL);
    printf("generate full deive list file\n");
    system(cmd_ls);
}

// check the path is file or directory
int is_file(char* path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

// display the list
void display(char* file, int cols)
{
    char line[300];
    char word[100];
    FILE *f = fopen(file, "r");
    int pivot1 = 0;
    int pivot2 = 0;
    int len = 0;
    int i = 0;
    int count = 0; // pivot count
    int flag = 0;
    if (cols == 1)
    {
        pivot1 = 40;
        printf(" ---------------------------------------\n");
        printf("| device name                           |\n");
        printf(" ---------------------------------------\n");
        while (fgets(line, sizeof(line), f) != NULL)
        {
            len = strlen(line);
            line[len - 1] = EOS;
            printf("| %s", line);
            for (i = len; i < pivot1 - 1; i++)
            {
                printf(" ");
            }
            printf("|\n");
            printf(" ---------------------------------------\n");
        }
    }
    if (cols == 3)
    {
        printf(" ");
        for (i = 0; i < max_length + 2; i++) printf("-");
        printf("\n");
        if (fgets(line, sizeof(line), f) != NULL)
        {
            for (i = 0; i < strlen(line); i++)
            {
                if (line[i] == ' ')
                {
                    flag = 0;
                    continue;
                }
                if (flag == 0 && line[i] != ' ')
                {
                    flag = 1;
                    if (count == 1) pivot1 = i;
                    if (count == 2) pivot2 = i;
                    count++;
                }
            }
        }
        fseek(f, 0, SEEK_SET);
        while (fgets(line, sizeof(line), f) != NULL)
        {
            printf("| ");
            strncpy(word, line, pivot1);
            word[pivot1 - 1] = EOS;
            printf("%s", word);
            printf("| ");
            if (pivot2 > pivot1) {
                strncpy(word, line + pivot1, pivot2 - pivot1);
                word[pivot2 - pivot1] = EOS;
                printf("%s", word);
                printf("| ");
                strncpy(word, line + pivot2, strlen(line) - pivot2 - 1);
                word[strlen(line) - pivot2 - 1] = EOS;
                printf("%s", word);
                
            } else
            {
                strncpy(word, line + pivot1, strlen(line) - pivot1 - 1);
                word[strlen(line) - pivot1 - 1] = EOS;
                printf("%s", word);
            }
            for (i = strlen(line); i < max_length - 1; i++) printf(" ");
            printf("|\n");
            printf(" ");
            for (i = 0; i < max_length + 2; i++) printf("-");
            printf("\n");
        }
    }
    fclose(f);
}

// romve specific file in the directory 
void remove_file(char* path, char* file) 
{
    char line[300];
    strcpy(line, "find ");
    int len = strlen(path);
    if (path[len - 1] == '/')
    {
        path[len - 1] = EOS;
    }
    strcat(line, path);
    strcat(line, " -name ");
    strcat(line, "\"");    
    strcat(line, file);
    strcat(line, "\" -type f -delete");
    system(line);
    printf("Clean completed.\n");    
}
