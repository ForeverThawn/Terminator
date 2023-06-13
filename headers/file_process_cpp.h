#define _CRT_SECURE_NO_WARNINGS
#include "forever.h"
#include <direct.h> 
#include <io.h>
#include <time.h>            // �����

typedef short mode;                  // ģʽ
enum createItemMode
{
    cEmpty = -1,     // ���ļ�
    cNormal = 0,     // ��ͨ���� ȫ�����ո�
    cSparse = 1,     // ���̷��� ָ��ϡ���ļ� ʮ��������� 00
    cRandom = 2,     // ������� ȫ����ASCII����ַ�

};

char* process;                       // �������̵Ľ�������
char path[MAX_PATH];                 // �����ļ�·��
char* fileName;                      // �����ļ���
char fullfileName[0x1000];           // �����ľ���·��
unsigned int processPid = 0;         // ����pid
//char* processName = 0;             // ��������
char* createdFileSize = 0;           // �������ļ���С (�ַ���)

bool fCreateItemSizeSet = false;       // �����ļ�ʱָ����С
bool fCreateItemNoDisplay = false;     // �����ļ�ʱ����ʾ״̬
bool fCreateItemModeSet = false;       // �����ļ�ʱָ��ģʽ

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
        if (_access(fullfileName, 0) == 0) // 0 �ɹ� -1 ʧ��  EACCES �ܾ�����   ENOENT δ�ҵ�·��   EINVAL ������Ч
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
            if (_access(fullfileName, 0) == 0) // 0 �ɹ� -1 ʧ��  EACCES �ܾ�����   ENOENT δ�ҵ�·��   EINVAL ������Ч
            {
                prints("ERROR: ", RED);
                prints("Cannot create file, file already existed!", RESET);
                return 1;
            }
            else
            {
                unsigned long long targetSize = 0; //size unit is byte

                for (int i = 0; i < strlen(createdFileSize); i++) // ������ȷ���ļ���С �����㵥λ��
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
        if (_access(fullfileName, 0) == 0) // 0 �ɹ� -1 ʧ��  EACCES �ܾ�����   ENOENT δ�ҵ�·��   EINVAL ������Ч
        {
            prints("ERROR: ", RED);
            prints("Cannot create file, file already existed!", RESET);
            return 1;
        }
        else
        {
            unsigned long long targetSize = 0; //size unit is byte

            for (int i = 0; i < strlen(createdFileSize); i++) // ������ȷ���ļ���С �����㵥λ��
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
            if (_access(fullfileName, 0) == 0) // 0 �ɹ� -1 ʧ��  EACCES �ܾ�����   ENOENT δ�ҵ�·��   EINVAL ������Ч
            {
                prints("ERROR: ", RED);
                prints("Cannot create file, file already existed!", RESET);
                return 1;
            }
            else
            {
                unsigned long long targetSize = 0; //size unit is byte

                for (int i = 0; i < strlen(createdFileSize); i++) // ������ȷ���ļ���С �����㵥λ��
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