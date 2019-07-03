//
// Created by song on 19-7-1.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "xycompress.h"


#define TOP_BIT_INT (0x80 << ((sizeof(int) -1) * 8))
#define INT_BIT_SIZE (sizeof(int) * 8)

typedef struct _node *Node;
typedef Node HuffmanTree;

struct _node {
    unsigned int charCode;
    unsigned int freq;
    Node parent;
    Node leftChild;
    Node rightChild;

    unsigned int val;
    unsigned int valLen;
};


void printBin(int num);

/**
 * 统计文件的字母频率
 * @param file
 * @param cfArray
 * @return
 */
int statCF(FILE *file, Node *const cfArray);

void arraySort(Node cfArray, int len);

void intCode2Str(int code, int len, char *result);

/**
 * 创建压缩字典树
 * @param huffmanTree
 * @param xyDict
 */
void buildXyDict(HuffmanTree huffmanTree);

void destroyHuffmanTree(HuffmanTree huffmanTree);


void preorder(HuffmanTree huffmanTree);

void saveDict(Node cf, int len, FILE *dictHandler);

void compressFile(Node cf, int len, const char *file, const char *destFile);

int buildDict(const char *dictFile, HuffmanTree *huffmanTree);

/**
 * 构建huffman树
 * @param orderedCf  从小到大排完序的频率数据
 * @param len 数据的长度
 * @return
 */
HuffmanTree buildHuffmanTree(Node orderedCf, int len);


void sortNodeArray(Node nodeArray[], int len);


int xy_compress(const char *file, const char *dictFile, const char *destFile) {
    FILE *fileHandler = fopen(file, "r");
    if (fileHandler == NULL) {
        return -1;
    }


    //1. 统计词频，并排序
    Node cf = NULL;
    int len = statCF(fileHandler, &cf);
    fclose(fileHandler);
    arraySort(cf, len);

    //2. 构建huffman树,顶点编码长度为0， 并给树的每个节点编码
    HuffmanTree tree = buildHuffmanTree(cf, len);
    tree->valLen = 0;
    buildXyDict(tree);

    //3.回收掉huffman树
    destroyHuffmanTree(tree);

    printf("代码为\n");
    for (int i = 0; i < len; i++) {
        char temp[cf[i].valLen + 1];
        intCode2Str(cf[i].val, cf[i].valLen, temp);
        printf("字符是： %c, 字符号是: %d, 编码是: %d, 编码长度是:%d, 字符串是: %s ;\n ", cf[i].charCode, cf[i].charCode, cf[i].val,
               cf[i].valLen, temp);
    }

    //4. 保存词频字典
    fileHandler = fopen(dictFile, "w");
    if (fileHandler == NULL) {
        return -1;
    }
    saveDict(cf, len, fileHandler);
    fclose(fileHandler);


    //5.压缩文件
    compressFile(cf, len, file, destFile);


    free(cf);
}


int xy_uncompress(const char *file, const char *dictFile, const char *destFile) {
    HuffmanTree tree = NULL;
    int codeLen = buildDict(dictFile, &tree);
    printf("词语的长度： %d", codeLen);


    FILE *sourceFileHandler = fopen(file, "r");
    if (sourceFileHandler == NULL) {
        return -1;
    }

    FILE *destFileHandler = fopen(destFile, "w");
    if (destFileHandler == NULL) {
        return -1;
    }

    int temp[2] = {0};
    int startCount = 0;


    int decodeTemp;
    Node findNode = tree;

    while (true) {
        int charTmp;
        size_t readsize = fread(&charTmp, sizeof(int), 1, sourceFileHandler);
        if (readsize == 0) {
            break;
        }

        printf("\n read int is: \n");
        printBin(charTmp);
        printf("\n");

        if (startCount > 1) {
            decodeTemp = temp[0];

            temp[0] = temp[1];
            temp[1] = charTmp;

        } else {
            temp[0] = temp[1];
            temp[1] = charTmp;
            startCount++;
            continue;
        }



        //write to file
        for (int i = 0; i < sizeof(int) * 8; i++) {
            if ((decodeTemp & TOP_BIT_INT) == TOP_BIT_INT) {
                findNode = findNode->rightChild;
            } else {
                findNode = findNode->leftChild;
            }

            decodeTemp = (decodeTemp << 1);

            if (findNode->leftChild == NULL) {
                fputc(findNode->charCode, destFileHandler);
                findNode = tree;
            }
        }
    }

    decodeTemp = temp[0];
    decodeTemp = (decodeTemp << (sizeof(int) * 8 - temp[1]));
    //write to file
    for (int i = 0; i < temp[1]; i++) {
        if ((decodeTemp & TOP_BIT_INT) == TOP_BIT_INT) {
            findNode = findNode->rightChild;
        } else {
            findNode = findNode->leftChild;
        }

        decodeTemp = (decodeTemp << 1);

        if (findNode->leftChild == NULL) {
            fputc(findNode->charCode, destFileHandler);
            findNode = tree;
        }
    }

    fclose(sourceFileHandler);
    fclose(destFileHandler);
}


int buildDict(const char *dictFile, HuffmanTree *huffmanTree) {
    FILE *fileHandler = fopen(dictFile, "r");
    if (fileHandler == NULL) {
        return -1;
    }

    char temp[10];
    char frequency[10];
    bool begin = true;
    int pos = 0;

    int kindCount = 0;

    Node node_temp = malloc(sizeof(struct _node) * INIT_WF_SIZE);
    int applyCount = INIT_WF_SIZE;


    while (true) {
        int charTmp = getc(fileHandler);
        if (charTmp == -1) {
            break;
        }

        if (charTmp == '\t') {
            temp[pos] = '\0';
            begin = false;
            pos = 0;
        } else if (charTmp == '\n') {
            frequency[pos] = '\0';
            begin = true;
            pos = 0;

            kindCount += 1;
            if (kindCount > applyCount) {
                applyCount += INC_WF_SIZE;
                node_temp = realloc(node_temp, sizeof(struct _node) * applyCount);
            }

            node_temp[kindCount - 1].charCode = atoi(temp);
            node_temp[kindCount - 1].freq = atoi(frequency);

        } else if (begin) {
            temp[pos++] = charTmp;
        } else {
            frequency[pos++] = charTmp;
        }

    }

    arraySort(node_temp, kindCount);
    //构建huffman树
    HuffmanTree tree = buildHuffmanTree(node_temp, kindCount);
    tree->valLen = 0;
    buildXyDict(tree);

    fclose(fileHandler);
    *huffmanTree = tree;
    return kindCount;
}


int statCF(FILE *file, Node *const cfArray) {
    int kindCount = 0;

    Node temp = malloc(sizeof(struct _node) * INIT_WF_SIZE);
    int applyCount = INIT_WF_SIZE;

    while (true) {
        int charTmp = getc(file);
        if (charTmp == -1) {
            break;
        }


        bool hasFindSameKind = false;
        for (int i = 0; i < kindCount; i++) {
            if (temp[i].charCode == charTmp) {
                temp[i].freq += 1;
                hasFindSameKind = true;
            }
        }

        if (!hasFindSameKind) {
            kindCount += 1;
            if (kindCount > applyCount) {
                applyCount += INC_WF_SIZE;
                temp = realloc(temp, sizeof(struct _node) * applyCount);
            }

            temp[kindCount - 1].charCode = charTmp;
            temp[kindCount - 1].freq = 1;
        }
    }

    *cfArray = temp;
    return kindCount;
}


void arraySort(Node cfArray, int len) {
    int tempCharCode;
    int tempFreq;

    for (int i = 0; i < len; i++) {
        for (int j = i; j < len; j++) {
            if (cfArray[i].freq > cfArray[j].freq) {
                tempCharCode = cfArray[i].charCode;
                tempFreq = cfArray[i].freq;

                cfArray[i].charCode = cfArray[j].charCode;
                cfArray[i].freq = cfArray[j].freq;

                cfArray[j].charCode = tempCharCode;
                cfArray[j].freq = tempFreq;

            }
        }
    }
}


HuffmanTree buildHuffmanTree(Node orderedCf, int len) {
    Node nodeArray[len];

    for (int i = 0; i < len; i++) {
        nodeArray[i] = orderedCf + i;
    }


    for (int i = 0; i < len - 1; i++) {
        Node first = nodeArray[0];
        Node second = nodeArray[1];

        nodeArray[0] = calloc(1, sizeof(struct _node));
        nodeArray[0]->freq = first->freq + second->freq;

        first->parent = nodeArray[0];
        second->parent = nodeArray[0];

        //树的左边放小的， 树的右边放大的
        if (first->freq > second->freq) {
            nodeArray[0]->rightChild = first;
            nodeArray[0]->leftChild = second;

        } else {
            nodeArray[0]->leftChild = first;
            nodeArray[0]->rightChild = second;
        }

        nodeArray[1] = nodeArray[len - i - 1];
        nodeArray[len - i - 1] = NULL;
        sortNodeArray(nodeArray, len - i - 1);
    }

    return nodeArray[0];
}


void sortNodeArray(Node nodeArray[], int len) {
    Node temp;

    for (int i = 0; i < len; i++) {
        for (int j = i; j < len; j++) {
            if (nodeArray[i]->freq > nodeArray[j]->freq) {
                temp = nodeArray[i];
                nodeArray[i] = nodeArray[j];
                nodeArray[j] = temp;
            }

        }
    }
}


void buildXyDict(HuffmanTree huffmanTree) {
    preorder(huffmanTree);
}

void preorder(HuffmanTree huffmanTree) {
    if (huffmanTree->leftChild != NULL && huffmanTree->rightChild != NULL) {
        huffmanTree->leftChild->valLen = huffmanTree->valLen + 1;
        huffmanTree->rightChild->valLen = huffmanTree->valLen + 1;

        if (huffmanTree->leftChild->valLen == 1) {
            huffmanTree->leftChild->val = 0;
            huffmanTree->rightChild->val = 1;
        } else {
            huffmanTree->leftChild->val = (huffmanTree->val << 1) + 0;
            huffmanTree->rightChild->val = (huffmanTree->val << 1) + 1;
        }

        preorder(huffmanTree->leftChild);
        preorder(huffmanTree->rightChild);
    }
}


void intCode2Str(int code, int len, char *result) {
    for (int i = 0; i < len; i++) {
        result[len - i - 1] = ((code & 0x1) == 1) ? '1' : '0';
        code = code >> 1;
    }

    result[len] = '\0';
}

void destroyHuffmanTree(HuffmanTree huffmanTree) {
    if (huffmanTree->leftChild != NULL && huffmanTree->rightChild != NULL) {
        destroyHuffmanTree(huffmanTree->leftChild);
        destroyHuffmanTree(huffmanTree->rightChild);

        free(huffmanTree);
    }
}


void saveDict(Node cf, int len, FILE *dictHandler) {
    char temp[20];
    for (int i = 0; i < len; i++) {
        sprintf(temp, "%d\t%d\n", cf[i].charCode, cf[i].freq);
        fputs(temp, dictHandler);
    }
}

void compressFile(Node cf, int len, const char *file, const char *destFile) {
    FILE *sourceFileHandler = fopen(file, "r");
    if (sourceFileHandler == NULL) {
        return;
    }

    FILE *destFileHandler = fopen(destFile, "w");
    if (destFileHandler == NULL) {
        return;
    }

    unsigned int temp = 0;
    unsigned int andCode;
    unsigned int addTemp;
    struct _node curNode;
    int tempLen = 0;
    int sy = 0;
    int reserverLen = 0;

    while (true) {
        int charTmp = getc(sourceFileHandler);
        if (charTmp == -1) {
            break;
        }

        printf("read code is: %d\n", charTmp);

        bool hasFound = false;

        for (int i = 0; i < len; i++) {
            curNode = cf[i];

            if (curNode.charCode == charTmp) {
                printf("char code is %d, charTemp is %d\n", curNode.charCode, charTmp);

                if (tempLen + curNode.valLen < INT_BIT_SIZE) {
                    tempLen += curNode.valLen;
                    temp = (temp << curNode.valLen) + curNode.val;
                    printBin(temp);
                } else {
                    andCode = 0;

                    //保留的是低位,其长度是 匹配的代码长度- 要填充到int的长度
                    //要填充到int的长度 = int的bit长度 - 缓存的int的长度
                    reserverLen = curNode.valLen - (INT_BIT_SIZE - tempLen);

                    for (int j = 0; j < reserverLen; j++) {
                        andCode = (andCode << 1) + 1;
                    }


                    //取剩下的数暂存
                    sy = curNode.val & andCode;

                    //移除掉剩下的数
                    addTemp = curNode.val >> reserverLen;

                    printBin(addTemp);

                    //补足int
                    temp = (temp << (INT_BIT_SIZE - tempLen)) + addTemp;

                    printBin(temp);
                    printf("\n");

                    fwrite(&temp, sizeof(int), 1, destFileHandler);

                    //将保留的值 赋值给 temp
                    temp = sy;

                    //将保留的长度 赋值给temp变量的长度
                    tempLen = reserverLen;

                }

                hasFound = true;
                break;
            }
        }

        if (!hasFound) {
            fputs("ERROR, FOUND CHAR NOT IN DICT\n", destFileHandler);
            break;
        }

    }


    fwrite(&temp, sizeof(int), 1, destFileHandler);
    printBin(temp);
    fwrite(&tempLen, sizeof(int), 1, destFileHandler);
    printf("bin len is: %d", tempLen);

    fclose(sourceFileHandler);
    fclose(destFileHandler);
}


void printBin(int num) {
    char temp[INT_BIT_SIZE + 1];

    temp[INT_BIT_SIZE + 1] = '\0';

    for (int i = 0; i < INT_BIT_SIZE; i++) {
        temp[INT_BIT_SIZE - i - 1] = ((num & 0x1) == 1 ? '1' : '0');
        num = num >> 1;
    }

    printf("%s\n", temp);
}
