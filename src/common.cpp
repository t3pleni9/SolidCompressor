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
        return ret;
}

extern int fill_buffer(SOLID_DATA buffer, size_t buf_len) {
    buffer->in_len = 0;
    int readed = 0;
    do {
        readed = read( buffer->fd.in, (buffer->in_buffer + buffer->in_len), buf_len - buffer->in_len);
        if(readed <= 0) {
            return readed;
        } else {
            buffer->in_len += readed;
        }
    } while(buffer->in_len < buf_len);
    
    return readed;

}

#ifdef __cplusplus
}
#endif
