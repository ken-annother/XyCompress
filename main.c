#include <stdio.h>
#include "xycompress.h"

extern void printBin(int num);

int main() {

//    xy_compress(
//            "/home/song/CLionProjects/XyCompress/DEMO2.txt",
//            "/home/song/CLionProjects/XyCompress/DEMO.ydict",
//            "/home/song/CLionProjects/XyCompress/DEMO.yzip");

    xy_uncompress(
            "/home/song/CLionProjects/XyCompress/DEMO.yzip",
            "/home/song/CLionProjects/XyCompress/DEMO.ydict",
            "/home/song/CLionProjects/XyCompress/DEMO_source.txt"
            );

    return 0;
}