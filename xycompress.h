//
// Created by song on 19-7-1.
//

#ifndef XYCOMPRESS_XYCOMPRESS_H
#define XYCOMPRESS_XYCOMPRESS_H


#define INIT_WF_SIZE 50
#define INC_WF_SIZE 25


int xy_compress(const char* file, const char * dictFile, const char *destFile);


int xy_uncompress(const char* file, const char * dictFile, const char *destFile);

#endif //XYCOMPRESS_XYCOMPRESS_H
