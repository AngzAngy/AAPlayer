#include "jnilogger.h"
#include "image-util.h"
#include <stdlib.h>

namespace imageUtil {
void saveBufToFile(char* buf,int size, char* fileName){
    FILE *fp = fopen(fileName, "wb");
    if (!fp){
        LOGE("finction: %s--open file error: %s\n",__FUNCTION__,fileName);
        return;
    }
    fwrite(buf,sizeof(char),size, fp);
    fclose(fp);
}

}// end of namespace
