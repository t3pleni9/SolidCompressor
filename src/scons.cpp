#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "scons.h"


#ifdef __cplusplus
extern "C" {
#endif

char errorMsg[100];
int dfd = 0;
int ifd = 0;
unsigned int compressionLevel = 47;

//const unsigned int SEG_S = 550000000;

unsigned int generateHeader() {
    


}
unsigned int retrieveHeader(unsigned int headerWord);

extern int write_buf(int fd, const void *buf, int size) {
	int ret;
	int pos = 0;
	while (pos < size) {
		ret = write(fd, (char*)buf + pos, size - pos);
		if (ret < 0) {
			ret = -errno;
			fprintf(stderr, "ERROR: failed to dump stream. %s",
					strerror(-ret));
			goto out;
		} else if (!ret) {
			ret = -EIO;
			fprintf(stderr, "ERROR: failed to dump stream. %s",
					strerror(-ret));
			goto out;
		}
        
        pos += ret;        
	}
    
	ret = 0;
    out:
        if(fd == dfd) 
            netOut += size;
        return pos;
}

extern int refill_buffer(SOLID_DATA buffer, size_t buf_len) {
    int readed = 0;
    while(buffer->in_len < buf_len){
        readed = read( buffer->fd.in, (buffer->in_buffer + buffer->in_len), buf_len - buffer->in_len);
        if(readed <= 0) {
            if(buffer->fd.in == ifd) 
                netIn += buffer->in_len;
            return readed;
        } else {
            buffer->in_len += readed;
        }
    }
    
    if(buffer->fd.in == ifd) 
        netIn += buffer->in_len;
    return readed;

}

extern int fill_buffer(SOLID_DATA buffer, size_t buf_len) {
    buffer->in_len = 0;
    int readed = refill_buffer(buffer, buf_len);
    return readed;

}

extern SOLID_RESULT wait_for_finish(pthread_t t_th) {
    void* t_res = NULL;
    int ret     = pthread_join(t_th, &t_res );
    if (ret) {
        ret = -ret;
        fprintf(stderr, "ERROR: pthread_join failed: %s\n",
            strerror(-ret));
        return STH_ERROR;
    } 
          
    if (t_res ) {
        SOLID_RESULT retResult = *(SOLID_RESULT *)t_res ;
        return retResult;
    }
       
    return STH_DONE;
}



#ifdef __cplusplus
}
#endif
