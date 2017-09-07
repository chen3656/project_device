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
const char *DB = "/home/schen/utilities/VIS_P25_DEV.txt";  // change this directory for specific libraray
static int max_length = 0;
int main(int argc, char* argv[])
{
    FILE *ff,*fn,*fp, *fd;
    char path[200];
    char file[200];
    char fn_name[200];
    char fd_name[200];
    char fp_name[200];
    int count = 0;
    int flag = 0;
    char command;
    char input[20];
    while (1) 
    {
        system("clear");
        // get input:
        printf(" --------------- Project Device Utitly ----------------\n");
        printf("| 1) Generate Full Device List                         |\n");
        printf("| 2) Generate Project Device List                      |\n");
        printf("| 3) Display Full Device List                          |\n");
        printf("| 4) Display Project Device List                       |\n");
        printf(" ------------------------------------------------------\n");
        printf("| 5) Clean the Locks                                   |\n");
        printf("| 6) Clean the temp file                               |\n");
        printf(" ------------------------------------------------------\n");
        printf("| q) Press q to quit the utility                       |\n");
        printf(" ------------------------------------------------------\n");
        printf("Enter the utility number: ");
        scanf("%c", &command);
        printf("=======================================================\n");
        switch (command)
        {
            case '1':
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
                    fclose(ff);
                    path2file(path);
                    ff = fopen(FULL, "r");
                }
                printf("Output file name: ");
                scanf("%s", fd_name);
                get_list(ff, fd_name);
                flag = 1;
                fclose(ff);
                printf("Full Device List Generated.\n");
                break;
            case '2':
                printf("Generate Project Device List\n");     
                if (flag == 0) 
                { 
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
                        fclose(ff);
                        path2file(path);
                        ff = fopen(FULL, "r");
                    }
                    get_list(ff, DEVICE);
                    fd = fopen(DEVICE, "r");  
                    fclose(ff);
                } else fd = fopen(fd_name, "r");
                printf("Please input netlist file path: ");
                scanf("%s", fn_name);
                fn = fopen(fn_name, "r");
                printf("Please input project list file path: ");
                scanf("%s", fp_name);
                fp = fopen(fp_name, "w");
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
                int flag = read_file(fd, fn, fp);
                if (flag == -1) return -1;
                else if (flag == 1) count = 1;
                else count = 3;
                // close the opened file
                fclose(fd);
                fclose(fn);
                fclose(fp);
                break;
            case '3':
                display(fd_name, 1);
                break;
            case '4':
                if (count <= 0) 
                {
                   printf("Can't display project list file\n");
                   return -1;
                }
                display(fp_name, count);
                break;               
            case '5':
                printf("Clean all locks(*.cdslck).\n");
                printf("Please enter the library full path: ");
                scanf("%s", path);
                remove_file(path, "*.cdslck");
                break;
            case '6':
                printf("Clean all temp file.\n");
                printf("Please enter the file suffix (format: *.suffix): ");
                scanf("%s", file);
                printf("Please enter the full path: ");
                scanf("%s", path);
                remove_file(path, file);
                break;
            case 'q':
                printf("exit the utility.\n"); 
                return 0; 
            default:
                printf("Wrong command!\n");
                return -1;
        }
        getchar();
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
    fprintf(fd, "Device Name\n");
    fprintf(fd, "------------------------------\n");
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
    char c;           // current character
    int i = 0;        // index
    int flag = 0;     // flag to show whether we need to show description.
    int label = 1;
    FILE *fb;
    fseek(fn, 0, SEEK_END);
    f_size = ftell(fn);
    printf("Generating list...\n");
    printf("-------------------------------------------------------\n");
    if (f_size > 5000000)
        printf ("The file is too big, please wait for couple of minutes.\n");
    int device_exist = 0; // device exist flag
    printf("Do you want to add device descripition to the list? (y/n)");
    scanf("%s", line);
    if (line[0] != 'y') flag = 1;
    if (flag == 0) 
    {
        fb = fopen(DB, "r");
        if (fb == NULL)
        {
            perror("can't find description file.\n");
            return -1;
        }
    }
    if (flag == 0) 
    {
        if (fgets(line, sizeof(line), fb) != NULL)
        {
            fprintf(fp, "%s", line);
            fprintf(fp, "-----------------------------------------------------------------------------------------\n");
        } else 
        {
            printf("Can't find descriptions in file.\n");
            flag = 1;
        }   
    }else 
    {
        fprintf(fp, "Device_Name\n------------------------\n");
    }
    fgets(device, sizeof(device), fd);  // skip the first two line
    fgets(device, sizeof(device), fd);  
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
            if (flag == 0) 
            {
                fseek(fb, 0, SEEK_SET);
                fgets(line, sizeof(line), fb);
                get_desc(fb, fp, device);
            }
            else 
           {
                fprintf(fp, "%d. %s\n", label++, device);                   
           }
        }
    }
    if (flag == 0) fclose(fb);
    printf("list genreated!\n");
    return flag;
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
        fgets(line, sizeof(line), f);
        pivot1 = 40;
        printf(" ---------------------------------------\n");
        printf("| Device Name                           |\n");
        printf(" ---------------------------------------\n");
        while (fgets(line, sizeof(line), f) != NULL)
        {
            if(line[0] == '-') continue;
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
        if (fgets(line, sizeof(line), f) != NULL)
        {
            if (max_length < strlen(line)) max_length = strlen(line);
            printf(" ");
            for (i = 0; i <= max_length + 2; i++) printf("-");
            printf("\n");
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
            if(line[0] == '-') continue;
            if (pivot2 < strlen(line)) {
                printf("| ");
                strncpy(word, line, pivot1);
                word[pivot1 - 1] = EOS;
                printf("%s", word);
                printf("| ");
                strncpy(word, line + pivot1, pivot2 - pivot1);
                word[pivot2 - pivot1] = EOS;
                printf("%s", word);
                printf("| ");
                strncpy(word, line + pivot2, strlen(line) - pivot2 - 1);
                word[strlen(line) - pivot2 - 1] = EOS;
                printf("%s", word);
                for (i = strlen(line); i < max_length; i++) printf(" ");
            } else
            {
                //strncpy(word, line + pivot1, strlen(line) - pivot1 - 1);
                //word[strlen(line) - pivot1 - 1] = EOS;
                line[strlen(line) - 1] = ' ';
                printf("| %s", line);
                for (i = strlen(line); i < max_length + 2; i++) printf(" ");
            }
            printf("|\n");
            printf(" ");
            for (i = 0; i <= max_length+2; i++) printf("-");
            printf("\n");
        }
    }
    fclose(f);
}

// romve specific file in the directory
void remove_file(char* path, char* file)
{
    char confirm[20];
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
    strcat(line, "\" -type f");
    system(line);
    printf("Are you sure you want to delete these files?(y/n) ");
    scanf("%s", confirm);
    if (confirm[0] != 'y') return;
    strcat(line, " -delete");
    system(line);
    printf("Clean completed.\n");
}
