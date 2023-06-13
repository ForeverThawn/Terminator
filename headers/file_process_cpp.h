#define _CRT_SECURE_NO_WARNINGS
#include "forever.h"
#include <direct.h> 
#include <io.h>
#include <time.h>            // 随机数

typedef short mode;                  // 模式
enum createItemMode
{
    cEmpty = -1,     // 空文件
    cNormal = 0,     // 普通分配 全部填充空格
    cSparse = 1,     // 立刻分配 指定稀疏文件 十六进制填充 00
    cRandom = 2,     // 随机分配 全部填ASCII随机字符

};

char* process;                       // 结束进程的进程名儿
char path[MAX_PATH];                 // 创建文件路径
char* fileName;                      // 创建文件名
char fullfileName[0x1000];           // 创建的绝对路径
unsigned int processPid = 0;         // 进程pid
//char* processName = 0;             // 进程名字
char* createdFileSize = 0;           // 创建的文件大小 (字符串)

bool fCreateItemSizeSet = false;       // 创建文件时指定大小
bool fCreateItemNoDisplay = false;     // 创建文件时不显示状态
bool fCreateItemModeSet = false;       // 创建文件时指定模式

int CreateItem(mode cMode)
{
    FILE* fp;
    /*int content[] =
    {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
    };*/

    _getcwd(path, MAX_PATH);

    strcpy_s(fullfileName, path);
    strcat_s(fullfileName, "\\");
    strcat_s(fullfileName, fileName);

    if (cMode == cEmpty)
    {
        if (_access(fullfileName, 0) == 0) // 0 成功 -1 失败  EACCES 拒绝访问   ENOENT 未找到路径   EINVAL 参数无效
        {
            prints("ERROR: ", RED);
            prints("Cannot create file, file already existed!", RESET);
            return 1;
        }
        else
        {
            fp = fopen((const char*)fullfileName, "w+");
            fclose(fp);

            prints("File Created Successfully", GREEN);
            puts(" | size: 0 Byte");
            return 0;
        }
    }

    if (cMode == cNormal)
    {
        if (fCreateItemSizeSet == true)
        {
            if (_access(fullfileName, 0) == 0) // 0 成功 -1 失败  EACCES 拒绝访问   ENOENT 未找到路径   EINVAL 参数无效
            {
                prints("ERROR: ", RED);
                prints("Cannot create file, file already existed!", RESET);
                return 1;
            }
            else
            {
                unsigned long long targetSize = 0; //size unit is byte

                for (int i = 0; i < strlen(createdFileSize); i++) // 载入正确的文件大小 （计算单位）
                {
                    if (createdFileSize[i] == 'B')
                    {
                        break;
                    }
                    if (createdFileSize[i] == 'K')
                    {
                        targetSize *= 1024;
                        break;
                    }
                    if (createdFileSize[i] == 'M')
                    {
                        targetSize *= (1024 * 1024);
                        break;
                    }
                    if (createdFileSize[i] == 'G')
                    {
                        targetSize *= (1024 * 1024 * 1024);
                        break;
                    }
                    int number = createdFileSize[i] - '0';
                    targetSize = targetSize * 10 + number;
                }

                if (targetSize > 0x10000000000)
                {
                    prints("Error: ", RED);
                    prints("Specified size is too large, try the size below 1024GB (1,099,511,627,776 bytes)\n", RESET);
                    return 1;
                }

                fp = fopen((const char*)fullfileName, "w+");

                if (fCreateItemNoDisplay == true)
                {
                    for (unsigned long long i = 0; i < targetSize; i++)
                    {
                        fprintf(fp, "%c", ' ');
                        //fwrite(content, , 16, fp);
                    }
                }
                else
                {
                    for (unsigned long long i = 0; i < targetSize; i++)
                    {
                        fprintf(fp, "%c", ' ');
                        printf("Filled size: %13lld Byte(s) | Total size: %13lld Byte(s)\r", i, targetSize);
                    }
                }
                fclose(fp);

                prints("\nFile Created Successfully", GREEN);
                printf(" | size: %lld Byte\n", targetSize);
                return 0;
            }
        }
        return 0;
    }
    if (cMode == cSparse)
    {
        if (_access(fullfileName, 0) == 0) // 0 成功 -1 失败  EACCES 拒绝访问   ENOENT 未找到路径   EINVAL 参数无效
        {
            prints("ERROR: ", RED);
            prints("Cannot create file, file already existed!", RESET);
            return 1;
        }
        else
        {
            unsigned long long targetSize = 0; //size unit is byte

            for (int i = 0; i < strlen(createdFileSize); i++) // 载入正确的文件大小 （计算单位）
            {
                if (createdFileSize[i] == 'B')
                {
                    break;
                }
                if (createdFileSize[i] == 'K')
                {
                    targetSize *= 1024;
                    break;
                }
                if (createdFileSize[i] == 'M')
                {
                    targetSize *= (1024 * 1024);
                    break;
                }
                if (createdFileSize[i] == 'G')
                {
                    targetSize *= (1024 * 1024 * 1024);
                    break;
                }
                int number = createdFileSize[i] - '0';
                targetSize = targetSize * 10 + number;
            }

            if (targetSize > 0x10000000000)
            {
                prints("Error: ", RED);
                prints("Specified size is too large, try the size below 1024GB (1,099,511,627,776 bytes)\n", RESET);
                return 1;
            }

            char sTargetSize[32];
            _ltoa(targetSize, sTargetSize, 10);

            char cCmd[MAX_PATH] = "fsutil file createnew ";
            strcat_s(cCmd, fullfileName);  strcat_s(cCmd, " ");
            strcat_s(cCmd, sTargetSize);   strcat_s(cCmd, " >nul");

            //prints(cCmd, BLUE);
            system((const char*)cCmd);

            prints("File Created Successfully", GREEN);
            printf(" | size: %lld Byte\n", targetSize);
            return 0;
        }
    }
    if (cMode == cRandom)
    {
        _getcwd(path, MAX_PATH);

        strcpy_s(fullfileName, path);
        strcat_s(fullfileName, "\\");
        strcat_s(fullfileName, fileName);

        if (fCreateItemSizeSet == true)
        {
            if (_access(fullfileName, 0) == 0) // 0 成功 -1 失败  EACCES 拒绝访问   ENOENT 未找到路径   EINVAL 参数无效
            {
                prints("ERROR: ", RED);
                prints("Cannot create file, file already existed!", RESET);
                return 1;
            }
            else
            {
                unsigned long long targetSize = 0; //size unit is byte

                for (int i = 0; i < strlen(createdFileSize); i++) // 载入正确的文件大小 （计算单位）
                {
                    if (createdFileSize[i] == 'B')
                    {
                        break;
                    }
                    if (createdFileSize[i] == 'K')
                    {
                        targetSize *= 1024;
                        break;
                    }
                    if (createdFileSize[i] == 'M')
                    {
                        targetSize *= (1024 * 1024);
                        break;
                    }
                    if (createdFileSize[i] == 'G')
                    {
                        targetSize *= (1024 * 1024 * 1024);
                        break;
                    }
                    int number = createdFileSize[i] - '0';
                    targetSize = targetSize * 10 + number;
                }

                if (targetSize > 0x10000000000)
                {
                    prints("Error: ", RED);
                    prints("Specified size is too large, try the size below 1024GB (1,099,511,627,776 bytes)\n", RESET);
                    return 1;
                }

                char character;
                srand(time(NULL));

                fp = fopen((const char*)fullfileName, "w+");

                if (fCreateItemNoDisplay == true)
                {
                    for (unsigned long long i = 0; i < targetSize; i++)
                    {
                        character = rand() % 0x7FFF;
                        fprintf(fp, "%c", character);
                    }
                }
                else
                {
                    for (unsigned long long i = 0; i < targetSize; i++)
                    {
                        character = rand() % 0x7FFF;
                        fprintf(fp, "%c", character);
                        printf("Filled size: %13lld Byte(s) | Total size: %13lld Byte(s)\r", i, targetSize);
                    }
                }
                fclose(fp);

                prints("\nFile Created Successfully", GREEN);
                printf(" | total size: %lld Byte\n", targetSize);
                return 0;
            }
        }
        return 0;
    }
}